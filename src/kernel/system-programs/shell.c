#include "shell.h"
#include "keyboard.h"
#include "kprint.h"
#include "string.h"
#include "io.h"
#include "pmm.h"
#include "ramfs.h"
#include "process.h"
#include "vmm.h"
#include "timer.h"

#define SHELL_PROMPT "> "
#define SHELL_PROMPT_LENGTH 2
#define SHELL_BUFFER_SIZE 256
#define CHAR_WIDTH 16
#define CHAR_HEIGHT 16

static char command_buffer[SHELL_BUFFER_SIZE];
static uint8_t command_index = 0;
static uint16_t cursor_x = SHELL_PROMPT_LENGTH * CHAR_WIDTH;
static uint16_t cursor_y = 0;

// Forward declarations
static void handle_backspace(void);
static void handle_enter(void);
static void handle_tab(void);
static void move_cursor_horizontal(int16_t chars);
static void move_cursor_vertical(uint16_t lines);
static void execute_command(const char *command);
static void shell_print(const char *str, uint32_t color);
static void cmd_meminfo(void);
static void cmd_ls(void);

static void shell_print(const char *str, uint32_t color) {
    while (*str) {
        if (*str == '\n') {
            move_cursor_vertical(1);
            cursor_x = 0;
        } else {
            kprint((char[]){*str, '\0'}, cursor_x, cursor_y, color);
            cursor_x += CHAR_WIDTH;
            if (cursor_x >= 1024) {
                cursor_x = 0;
                move_cursor_vertical(1);
            }
        }
        str++;
    }
}

static void handle_backspace(void) {
    if (command_index > 0) {
        command_buffer[--command_index] = '\0';
        move_cursor_horizontal(-1);
        kprint(" ", cursor_x, cursor_y, 0x000000);
    }
}

static void handle_enter(void) {
    command_buffer[command_index] = '\0';
    move_cursor_vertical(1);
    cursor_x = 0;
    execute_command(command_buffer);

    memset(command_buffer, 0, sizeof(command_buffer));
    command_index = 0;

    move_cursor_vertical(1);
    cursor_x = 0;
    kprint(SHELL_PROMPT, 0, cursor_y, 0xCF9FFF);
    cursor_x = SHELL_PROMPT_LENGTH * CHAR_WIDTH;
}

static void handle_tab(void) {
    const char *tab_spaces = "    ";
    size_t tab_len = strlen(tab_spaces);
    if (command_index + tab_len < SHELL_BUFFER_SIZE) {
        strncpy(&command_buffer[command_index], tab_spaces, tab_len);
        command_index += tab_len;
        kprint(tab_spaces, cursor_x, cursor_y, 0xFFFFFF);
        move_cursor_horizontal(tab_len);
    }
}

static void move_cursor_horizontal(int16_t chars) {
    cursor_x += chars * CHAR_WIDTH;
}

static void move_cursor_vertical(uint16_t lines) {
    cursor_y += lines * CHAR_HEIGHT;
}

// --- Commands ---

static void cmd_help(void) {
    shell_print("commands:\n", 0xFFFFFF);
    shell_print("  help     - show this\n", 0x9966CC);
    shell_print("  ls       - list files\n", 0x9966CC);
    shell_print("  cat <f>  - read file\n", 0x9966CC);
    shell_print("  exec <f> - run program\n", 0x9966CC);
    shell_print("  meminfo  - memory stats\n", 0x9966CC);
    shell_print("  ps       - process list\n", 0x9966CC);
    shell_print("  uptime   - tick count\n", 0x9966CC);
    shell_print("  clear    - clear screen\n", 0x9966CC);
    shell_print("  hello    - :)\n", 0x9966CC);
    shell_print("  reboot   - reset\n", 0x9966CC);
}

static void cmd_ls(void) {
    char buf[1024];
    ramfs_list(buf, sizeof(buf));
    shell_print(buf, 0xCF9FFF);
}

static void cmd_cat(const char *filename) {
    // skip whitespace
    while (*filename == ' ') filename++;

    ramfs_file_t *file = ramfs_open(filename);
    if (!file) {
        shell_print("file not found: ", 0xFF4466);
        shell_print(filename, 0xFF4466);
        shell_print("\n", 0xFF4466);
        return;
    }

    char buf[512];
    size_t to_read = file->size < sizeof(buf) - 1 ? file->size : sizeof(buf) - 1;
    int n = ramfs_read(file, (uint8_t *)buf, 0, to_read);
    if (n > 0) {
        buf[n] = '\0';
        shell_print(buf, 0xCCCCCC);
    }
}

static void cmd_exec(const char *filename) {
    while (*filename == ' ') filename++;

    ramfs_file_t *file = ramfs_open(filename);
    if (!file) {
        shell_print("file not found: ", 0xFF4466);
        shell_print(filename, 0xFF4466);
        shell_print("\n", 0xFF4466);
        return;
    }

    if (!file->executable) {
        shell_print("not executable: ", 0xFF4466);
        shell_print(filename, 0xFF4466);
        shell_print("\n", 0xFF4466);
        return;
    }

    shell_print("loading ", 0xCF9FFF);
    shell_print(filename, 0xCF9FFF);
    shell_print("...\n", 0xCF9FFF);

    // For user-mode execution, we need to:
    // 1. Create a new address space
    // 2. Map the program code at USER_CODE_BASE
    // 3. Create process with entry at USER_CODE_BASE

    uint64_t *new_pml4 = vmm_create_address_space();
    if (!new_pml4) {
        shell_print("failed to create address space\n", 0xFF4466);
        return;
    }

    // Map program pages
    extern uint64_t hhdm_offset;
    uint64_t *pml4_virt = (uint64_t *)((uint64_t)new_pml4 + hhdm_offset);
    size_t pages_needed = (file->size + 4095) / 4096;
    for (size_t i = 0; i < pages_needed; i++) {
        void *page = pmm_alloc_page();
        if (!page) {
            shell_print("out of memory\n", 0xFF4466);
            return;
        }
        vmm_map_page(pml4_virt, USER_CODE_BASE + i * 4096, (uint64_t)page,
                     VMM_FLAG_PRESENT | VMM_FLAG_WRITE | VMM_FLAG_USER);

        // Copy program data to this page
        void *page_virt = (void *)((uint64_t)page + hhdm_offset);
        size_t offset = i * 4096;
        size_t copy_len = file->size - offset;
        if (copy_len > 4096) copy_len = 4096;
        memcpy(page_virt, file->data + offset, copy_len);
    }

    // Create process (the process_create will set up its own address space,
    // but we've prepared one — we need to assign it)
    process_t *proc = process_create(filename, (void (*)(void))USER_CODE_BASE, 1);
    if (!proc) {
        shell_print("failed to create process\n", 0xFF4466);
        return;
    }

    // Override with our prepared address space that has code mapped
    // First, copy kernel half to our new pml4
    uint64_t *kernel_pml4_virt = (uint64_t *)((uint64_t)vmm_get_kernel_pml4() + hhdm_offset);
    for (int i = 256; i < 512; i++) {
        pml4_virt[i] = kernel_pml4_virt[i];
    }

    // Copy user stack mappings from the process's auto-created address space to ours
    // Actually, let's just map user stack in our new pml4 too
    void *ustack1 = pmm_alloc_page();
    void *ustack2 = pmm_alloc_page();
    if (ustack1 && ustack2) {
        uint64_t ustack_base = USER_STACK_BASE - 4096 * 2;
        vmm_map_page(pml4_virt, ustack_base, (uint64_t)ustack1,
                     VMM_FLAG_PRESENT | VMM_FLAG_WRITE | VMM_FLAG_USER);
        vmm_map_page(pml4_virt, ustack_base + 4096, (uint64_t)ustack2,
                     VMM_FLAG_PRESENT | VMM_FLAG_WRITE | VMM_FLAG_USER);
    }

    proc->page_table = new_pml4;

    shell_print("process created: pid ", 0xCF9FFF);
    char pidbuf[8];
    int pid = proc->pid;
    int pi = 0;
    if (pid >= 10) pidbuf[pi++] = '0' + (pid / 10);
    pidbuf[pi++] = '0' + (pid % 10);
    pidbuf[pi] = '\0';
    shell_print(pidbuf, 0xCF9FFF);
    shell_print("\n", 0xCF9FFF);
}

static void cmd_meminfo(void) {
    char buf[16];

    shell_print("physical memory:\n", 0xFFFFFF);

    shell_print("  total pages: ", 0x9966CC);
    uint64_t total = pmm_get_total_pages();
    itoa((int)total, buf, 10);
    shell_print(buf, 0xCF9FFF);
    shell_print("\n", 0xCF9FFF);

    shell_print("  used pages:  ", 0x9966CC);
    uint64_t used = pmm_get_used_pages();
    itoa((int)used, buf, 10);
    shell_print(buf, 0xCF9FFF);
    shell_print("\n", 0xCF9FFF);

    shell_print("  free pages:  ", 0x9966CC);
    uint64_t free_p = pmm_get_free_pages();
    itoa((int)free_p, buf, 10);
    shell_print(buf, 0xCF9FFF);
    shell_print("\n", 0xCF9FFF);

    shell_print("  total RAM:   ", 0x9966CC);
    uint64_t total_mem = pmm_get_total_memory();
    itoa((int)(total_mem / (1024 * 1024)), buf, 10);
    shell_print(buf, 0xCF9FFF);
    shell_print(" MB\n", 0xCF9FFF);
}

static void cmd_ps(void) {
    char buf[1024];
    extern int process_list(char *buf, size_t bufsize);
    shell_print("PID STATE NAME\n", 0xFFFFFF);
    process_list(buf, sizeof(buf));
    shell_print(buf, 0xCF9FFF);
}

static void cmd_uptime(void) {
    char buf[16];
    uint64_t t = timer_get_ticks();
    itoa((int)(t / 1000), buf, 10);
    shell_print("uptime: ", 0x9966CC);
    shell_print(buf, 0xCF9FFF);
    shell_print("s\n", 0xCF9FFF);
}

static void cmd_clear(void) {
    extern uint32_t *framebuffer_ptr;
    extern size_t framebuffer_width, framebuffer_height;
    for (size_t i = 0; i < framebuffer_width * framebuffer_height; i++) {
        framebuffer_ptr[i] = 0x000000;
    }
    cursor_x = 0;
    cursor_y = 0;
}

static void execute_command(const char *command) {
    if (strlen(command) == 0)
        return;

    if (strcmp(command, "hello") == 0) {
        shell_print("Hello, world!\n", 0xCF9FFF);
    } else if (strcmp(command, "help") == 0) {
        cmd_help();
    } else if (strcmp(command, "ls") == 0) {
        cmd_ls();
    } else if (strncmp(command, "cat ", 4) == 0) {
        cmd_cat(command + 4);
    } else if (strncmp(command, "exec ", 5) == 0) {
        cmd_exec(command + 5);
    } else if (strcmp(command, "meminfo") == 0) {
        cmd_meminfo();
    } else if (strcmp(command, "ps") == 0) {
        cmd_ps();
    } else if (strcmp(command, "uptime") == 0) {
        cmd_uptime();
    } else if (strcmp(command, "clear") == 0) {
        cmd_clear();
    } else if (strcmp(command, "reboot") == 0) {
        shell_print("rebooting...\n", 0xFF4466);
        // Triple fault to reboot
        __asm__ volatile("lidt (%%rax)" : : "a"(0));
    } else if (strcmp(command, "poweroff") == 0) {
        shell_print("shutting down...\n", 0xFF4466);
        extern void acpi_shutdown(void);
        acpi_shutdown();
    } else {
        shell_print("unknown: ", 0xFF4466);
        shell_print(command, 0xFF4466);
        shell_print("\n", 0xFF4466);
    }
}

void shell_init(void) {
    /* Print MOTD */
    ramfs_file_t *motd = ramfs_open("motd.txt");
    if (motd) {
        char buf[256];
        int n = ramfs_read(motd, (uint8_t *)buf, 0, sizeof(buf) - 1);
        if (n > 0) {
            buf[n] = '\0';
            shell_print(buf, 0xCF9FFF);
        }
    }

    /* Auto-display system info on boot */
    cmd_meminfo();
    move_cursor_vertical(1);
    shell_print("files:\n", 0xFFFFFF);
    cmd_ls();

    move_cursor_vertical(1);
    kprint(SHELL_PROMPT, 0, cursor_y, 0xCF9FFF);
    command_index = 0;
    cursor_x = SHELL_PROMPT_LENGTH * CHAR_WIDTH;
}

void shell_run(void) {
    while (1) {
        for (uint8_t scancode = 0; scancode < KEY_STATE_SIZE; ++scancode) {
            if (is_key_pressed(scancode)) {
                char key = scancode_to_char(scancode);
                switch (key) {
                    case '\b':
                        handle_backspace();
                        break;
                    case '\n':
                        handle_enter();
                        break;
                    case '\t':
                        handle_tab();
                        break;
                    default:
                        if (command_index < SHELL_BUFFER_SIZE - 1) {
                            command_buffer[command_index++] = key;
                            command_buffer[command_index] = '\0';
                            char tmp[2] = {key, '\0'};
                            kprint(tmp, cursor_x, cursor_y, 0xFFFFFF);
                            move_cursor_horizontal(1);
                        }
                        break;
                }
                update_key_state(scancode, 0);
            }
        }
        __asm__ volatile("hlt");  // Wait for next interrupt
    }
}
