bits 32

global _start
extern kernel_main

section .text

_start:

    cli

    ; Stack do kernel
    mov esp, 0x90000

    call kernel_main


hang:
    cli
    hlt
    jmp hang
