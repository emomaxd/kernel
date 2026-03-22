# Kernel Project Roadmap

## Current State
- Limine bootloader (x86-64, higher half at 0xffffffff80000000)
- GDT + TSS (user segments defined but unused)
- IDT + PIC (256 vectors, IRQ framework)
- Keyboard driver (IRQ1)
- Framebuffer rendering + basic shell
- Buddy allocator on static 1MB heap
- No physical memory management, no paging, no processes

## Phase 1: Physical Memory Manager (PMM)
**Goal:** Manage real physical RAM using Limine's memory map.

- Parse `limine_memmap_request` to discover usable RAM regions
- Bitmap-based page frame allocator (4KB pages)
- `pmm_init()`, `pmm_alloc_page()`, `pmm_free_page()`
- Track total/used/free page counts

## Phase 2: Virtual Memory Manager (VMM)
**Goal:** Take over paging from Limine, full control of address space.

- 4-level page table management (PML4 → PDPT → PD → PT)
- `vmm_init()` — create kernel page tables, identity map + higher half map
- `vmm_map_page(virt, phys, flags)`, `vmm_unmap_page(virt)`
- Load own CR3, replacing Limine's page tables
- Kernel heap upgrade: `kmalloc()`/`kfree()` backed by VMM pages

## Phase 3: PIT Timer
**Goal:** Preemptive scheduling foundation.

- Program PIT channel 0 at ~1000 Hz (1ms tick)
- IRQ0 handler with global tick counter
- `timer_get_ticks()`, `timer_sleep_ticks(n)` (busy-wait)

## Phase 4: Process Management & Context Switching
**Goal:** Run a user-mode task (ring 3).

- Process Control Block (PCB): pid, state, registers, page table, kernel stack
- Context switch via `switch_context()` in assembly
- TSS rsp0 set per-process for ring 3 → ring 0 transitions
- Round-robin scheduler triggered by timer IRQ
- `process_create()`, `process_exit()`, `schedule()`

## Phase 5: System Call Interface
**Goal:** User-mode programs can request kernel services.

- `syscall`/`sysret` (MSR-based) or `int 0x80` gate
- Syscall dispatch table
- Initial syscalls: `sys_write`, `sys_exit`, `sys_getpid`

## Phase 6: RAM Filesystem
**Goal:** In-memory filesystem for shell interaction.

- Simple ramfs: flat directory with file entries (name, data pointer, size)
- `ramfs_init()` — populate with built-in files
- `ramfs_open()`, `ramfs_read()`, `ramfs_list()`
- No disk driver needed — memory-backed

## Phase 7: Shell Integration & User Programs
**Goal:** Shell can list files, read them, and execute programs.

- Shell commands: `ls`, `cat <file>`, `exec <program>`, `meminfo`, `ps`
- User-mode ELF loader (simple flat binary for now)
- Built-in test programs embedded in kernel image
- Demonstrate full boot → shell → run user program → return

## Architecture Overview
```
┌─────────────────────────────────────────┐
│  User Programs (Ring 3)                 │
├─────────────────────────────────────────┤
│  Syscall Interface                      │
├─────────────────────────────────────────┤
│  Process Manager  │  RAM Filesystem     │
├───────────────────┼─────────────────────┤
│  Scheduler (PIT)  │  Shell              │
├───────────────────┴─────────────────────┤
│  Virtual Memory Manager (VMM)           │
├─────────────────────────────────────────┤
│  Physical Memory Manager (PMM)          │
├─────────────────────────────────────────┤
│  GDT/TSS │ IDT/PIC │ Keyboard │ Timer  │
├─────────────────────────────────────────┤
│  Limine Bootloader                      │
└─────────────────────────────────────────┘
```
