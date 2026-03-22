#ifndef PROCESS_H
#define PROCESS_H

#include <stdint.h>
#include <stddef.h>

#define MAX_PROCESSES     64
#define KERNEL_STACK_SIZE 8192
#define USER_STACK_SIZE   16384
#define USER_STACK_BASE   0x00007FFFFFFFE000ULL
#define USER_CODE_BASE    0x0000000000400000ULL

typedef enum {
    PROCESS_STATE_UNUSED = 0,
    PROCESS_STATE_READY,
    PROCESS_STATE_RUNNING,
    PROCESS_STATE_BLOCKED,
    PROCESS_STATE_ZOMBIE
} process_state_t;

typedef struct {
    uint64_t r15, r14, r13, r12;
    uint64_t rbp, rbx;
    uint64_t rip;
} context_t;

typedef struct {
    uint32_t pid;
    process_state_t state;
    char name[32];

    // CPU context (saved on switch)
    context_t *context;

    // Memory
    uint64_t *page_table;    // PML4
    uint64_t kernel_stack;   // top of kernel stack
    uint64_t kernel_stack_base; // base allocation
    uint64_t user_stack;

    // Ring 3 entry
    uint64_t entry_point;
} process_t;

void process_init(void);
process_t *process_create(const char *name, void (*entry)(void), int user_mode);
void process_exit(void);
process_t *process_get_current(void);

void schedule(void);
void scheduler_tick(void);

// Assembly
extern void switch_context(context_t **old, context_t *new_ctx);
extern void jump_to_usermode(uint64_t entry, uint64_t stack, uint64_t cs, uint64_t ds);

#endif // PROCESS_H
