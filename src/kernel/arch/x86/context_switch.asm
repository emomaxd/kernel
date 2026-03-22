[bits 64]
section .text

; void switch_context(context_t **old, context_t *new_ctx)
; rdi = &old->context (pointer to pointer)
; rsi = new->context
global switch_context
switch_context:
    ; Save callee-saved registers on current stack
    push rbp
    push rbx
    push r12
    push r13
    push r14
    push r15

    ; Save current RSP into old context pointer
    mov [rdi], rsp

    ; Load new RSP from new context
    mov rsp, rsi

    ; Restore callee-saved registers
    pop r15
    pop r14
    pop r13
    pop r12
    pop rbx
    pop rbp

    ; Return to new task's RIP (already on stack from initial setup or previous switch)
    ret

; void jump_to_usermode(uint64_t entry, uint64_t stack, uint64_t cs, uint64_t ds)
; rdi = user entry point (RIP)
; rsi = user stack pointer (RSP)
; rdx = user code segment selector
; rcx = user data segment selector
global jump_to_usermode
jump_to_usermode:
    ; Set data segments
    mov ax, cx
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    ; Build iretq stack frame:
    ; SS, RSP, RFLAGS, CS, RIP
    push rcx        ; SS (user data segment)
    push rsi        ; RSP (user stack)
    pushfq
    pop rax
    or rax, 0x200   ; Set IF (interrupt flag)
    push rax        ; RFLAGS
    push rdx        ; CS (user code segment)
    push rdi        ; RIP (entry point)

    iretq
