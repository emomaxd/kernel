[org 0x7C00]

start:
    mov si, message
print:
    lodsb
    cmp al, 0
    je load_kernel
    mov ah, 0x0E
    int 0x10
    jmp print

load_kernel:
    ; Load the kernel (which is at sector 2) into memory at address 0x1000
    mov ax, 0x1000      ; Set segment to 0x1000
    mov es, ax
    mov bx, 0x0000      ; Set offset to 0x0000 (0x1000:0x0000 = 0x10000 physical address)
    mov ah, 0x02        ; BIOS read sectors function
    mov al, 1           ; Read 1 sector (512 bytes)
    mov ch, 0           ; Cylinder 0
    mov cl, 2           ; Sector 2 (kernel starts here)
    mov dh, 0           ; Head 0
    mov dl, 0           ; Drive 0 (first floppy or hard drive)
    int 0x13            ; BIOS interrupt to read sector

    ; Jump to the kernel at 0x1000:0x0000
    jmp 0x1000:0x0000

message db 'Loading Kernel...', 0

times 510-($-$$) db 0
dw 0xAA55
