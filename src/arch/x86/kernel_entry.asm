section .text
global _start

_start:
    ; Initialize your kernel here
    ; You might want to call your `main` function or setup the environment

    ; Example of calling a function (if needed):
    [extern main]
    call main

    ; Infinite loop to keep the system running
    hlt
    jmp $
