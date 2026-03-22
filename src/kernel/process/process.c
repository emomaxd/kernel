#include "process.h"
#include "vmm.h"
#include "pmm.h"
#include "gdt.h"
#include "string.h"
#include "io.h"

extern uint64_t hhdm_offset;

static process_t process_table[MAX_PROCESSES];
static uint32_t next_pid = 1;
static int current_process = -1;
static int scheduler_enabled = 0;

void process_init(void) {
    memset(process_table, 0, sizeof(process_table));

    // Create idle process (pid 0) — runs in kernel mode
    process_t *idle = &process_table[0];
    idle->pid = 0;
    idle->state = PROCESS_STATE_RUNNING;
    strncpy(idle->name, "idle", 31);
    idle->page_table = vmm_get_kernel_pml4();

    current_process = 0;
    scheduler_enabled = 1;
}

// Wrapper that sets up the stack frame for a new kernel task
static void kernel_task_entry(void) {
    process_t *proc = process_get_current();
    if (proc && proc->entry_point) {
        ((void (*)(void))proc->entry_point)();
    }
    process_exit();
}

// Wrapper for user-mode task entry
static void user_task_entry(void) {
    process_t *proc = process_get_current();
    if (!proc)
        return;

    // Set TSS rsp0 so interrupts in ring 3 use this kernel stack
    tss_set_rsp0(proc->kernel_stack);

    // Jump to user mode
    jump_to_usermode(proc->entry_point, proc->user_stack,
                     0x18 | 3,   // user code segment | RPL 3
                     0x20 | 3);  // user data segment | RPL 3
}

process_t *process_create(const char *name, void (*entry)(void), int user_mode) {
    // Find free slot
    int slot = -1;
    for (int i = 1; i < MAX_PROCESSES; i++) {
        if (process_table[i].state == PROCESS_STATE_UNUSED) {
            slot = i;
            break;
        }
    }
    if (slot < 0)
        return NULL;

    process_t *proc = &process_table[slot];
    memset(proc, 0, sizeof(process_t));

    proc->pid = next_pid++;
    proc->state = PROCESS_STATE_READY;
    strncpy(proc->name, name, 31);
    proc->entry_point = (uint64_t)entry;

    // Allocate kernel stack (2 pages)
    void *kstack_phys = pmm_alloc_page();
    void *kstack_phys2 = pmm_alloc_page();
    if (!kstack_phys || !kstack_phys2)
        return NULL;

    uint64_t kstack_virt = (uint64_t)kstack_phys + hhdm_offset;
    // Map the second page right after
    uint64_t kstack_virt2 = (uint64_t)kstack_phys2 + hhdm_offset;
    (void)kstack_virt2; // used via contiguous HHDM

    proc->kernel_stack_base = kstack_virt;
    proc->kernel_stack = kstack_virt + PAGE_SIZE;  // top of first page

    if (user_mode) {
        // Create separate address space
        proc->page_table = vmm_create_address_space();
        if (!proc->page_table)
            return NULL;

        // Allocate and map user stack
        void *ustack_phys = pmm_alloc_page();
        void *ustack_phys2 = pmm_alloc_page();
        if (!ustack_phys || !ustack_phys2)
            return NULL;

        uint64_t *pml4_virt = (uint64_t *)((uint64_t)proc->page_table + hhdm_offset);
        uint64_t ustack_base = USER_STACK_BASE - PAGE_SIZE * 2;
        vmm_map_page(pml4_virt, ustack_base, (uint64_t)ustack_phys,
                     VMM_FLAG_PRESENT | VMM_FLAG_WRITE | VMM_FLAG_USER);
        vmm_map_page(pml4_virt, ustack_base + PAGE_SIZE, (uint64_t)ustack_phys2,
                     VMM_FLAG_PRESENT | VMM_FLAG_WRITE | VMM_FLAG_USER);
        proc->user_stack = USER_STACK_BASE;
    } else {
        proc->page_table = vmm_get_kernel_pml4();
    }

    // Set up kernel stack with initial context
    // The context is placed at the top of kernel stack
    uint64_t sp = proc->kernel_stack;
    sp -= sizeof(context_t);
    context_t *ctx = (context_t *)sp;
    memset(ctx, 0, sizeof(context_t));

    if (user_mode) {
        ctx->rip = (uint64_t)user_task_entry;
    } else {
        ctx->rip = (uint64_t)kernel_task_entry;
    }
    ctx->rbp = 0;

    proc->context = ctx;

    return proc;
}

void process_exit(void) {
    process_t *proc = process_get_current();
    if (proc && proc->pid != 0) {
        proc->state = PROCESS_STATE_ZOMBIE;
    }
    schedule();
    // Should not return
    for (;;) {
        __asm__ volatile("hlt");
    }
}

process_t *process_get_current(void) {
    if (current_process < 0 || current_process >= MAX_PROCESSES)
        return NULL;
    return &process_table[current_process];
}

void schedule(void) {
    if (!scheduler_enabled)
        return;

    int old = current_process;

    // Simple round-robin: find next READY process
    int next = old;
    for (int i = 1; i <= MAX_PROCESSES; i++) {
        int idx = (old + i) % MAX_PROCESSES;
        if (process_table[idx].state == PROCESS_STATE_READY) {
            next = idx;
            break;
        }
    }

    if (next == old)
        return;

    process_t *old_proc = &process_table[old];
    process_t *new_proc = &process_table[next];

    if (old_proc->state == PROCESS_STATE_RUNNING)
        old_proc->state = PROCESS_STATE_READY;
    new_proc->state = PROCESS_STATE_RUNNING;
    current_process = next;

    // Set TSS rsp0 for the new process
    tss_set_rsp0(new_proc->kernel_stack);

    // Switch address space if different
    if (old_proc->page_table != new_proc->page_table) {
        vmm_switch_address_space(new_proc->page_table);
    }

    // Context switch
    switch_context(&old_proc->context, new_proc->context);
}

void scheduler_tick(void) {
    // Called from timer IRQ — triggers scheduling
    // For now, schedule on every tick (1ms)
    // A real scheduler would use time slices
    static uint64_t tick_count = 0;
    tick_count++;
    // Schedule every 10ms (10 ticks at 1000Hz)
    if (tick_count % 10 == 0) {
        schedule();
    }
}

// Get process list info for 'ps' command
int process_list(char *buf, size_t bufsize) {
    int pos = 0;
    for (int i = 0; i < MAX_PROCESSES && (size_t)pos < bufsize - 1; i++) {
        if (process_table[i].state != PROCESS_STATE_UNUSED) {
            const char *state_str = "???";
            switch (process_table[i].state) {
                case PROCESS_STATE_READY:   state_str = "READY"; break;
                case PROCESS_STATE_RUNNING: state_str = "RUN  "; break;
                case PROCESS_STATE_BLOCKED: state_str = "BLOCK"; break;
                case PROCESS_STATE_ZOMBIE:  state_str = "DEAD "; break;
                default: break;
            }
            // Manual formatting: "PID STATE NAME\n"
            // PID
            uint32_t pid = process_table[i].pid;
            if (pid >= 10) buf[pos++] = '0' + (pid / 10);
            buf[pos++] = '0' + (pid % 10);
            buf[pos++] = ' ';
            // State
            for (int j = 0; state_str[j] && (size_t)pos < bufsize - 1; j++)
                buf[pos++] = state_str[j];
            buf[pos++] = ' ';
            // Name
            for (int j = 0; process_table[i].name[j] && (size_t)pos < bufsize - 1; j++)
                buf[pos++] = process_table[i].name[j];
            buf[pos++] = '\n';
        }
    }
    buf[pos] = '\0';
    return pos;
}
