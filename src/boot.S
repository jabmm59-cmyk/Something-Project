/* Multiboot2 entry (32-bit), switch to long mode, call kmain. GNU as, Intel syntax. */
.intel_syntax noprefix

.section .multiboot2, "a"
.align 8
mb2_start:
    .long 0xE85250D6
    .long 0
    .long mb2_end - mb2_start
    .long 0x100000000 - (0xE85250D6 + 0 + (mb2_end - mb2_start))
    .align 8
    .short 0
    .short 0
    .long 8
mb2_end:

.section .bss
.align 4096
p4_table: .skip 4096
p3_table: .skip 4096
p2_table: .skip 4096
.align 16
stack_bottom: .skip 16384
stack_top:

.section .rodata
.align 8
gdt64:
    .quad 0
    .quad 0x00AF9A000000FFFF
    .quad 0x00AF92000000FFFF
gdt64_end:
gdt64_ptr:
    .word gdt64_end - gdt64 - 1
    .long gdt64

.section .text
.code32
.global _start
_start:
    cli
    mov esp, offset stack_top
    cmp eax, 0x36d76289
    jne no_multiboot

    mov eax, offset p3_table
    or eax, 3
    mov dword ptr [p4_table], eax
    mov eax, offset p2_table
    or eax, 3
    mov dword ptr [p3_table], eax

    xor ecx, ecx
1:
    mov eax, 0x200000
    mul ecx
    or eax, 0x83
    mov dword ptr [p2_table + ecx*8], eax
    inc ecx
    cmp ecx, 512
    jne 1b

    mov eax, offset p4_table
    mov cr3, eax
    mov eax, cr4
    or eax, 0x20
    mov cr4, eax
    mov ecx, 0xC0000080
    rdmsr
    or eax, 0x100
    wrmsr
    mov eax, cr0
    or eax, 0x80000000
    mov cr0, eax

    lgdt [gdt64_ptr]
    .byte 0xEA
    .long long_mode_start
    .word 0x08

no_multiboot:
    mov dword ptr [0xB8000], 0x4F454F45
    hlt

.code64
long_mode_start:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov fs, ax
    mov gs, ax
    lea rsp, [rip + stack_top]
    call kmain
halt_loop:
    cli
    hlt
    jmp halt_loop

.section .note.GNU-stack, "", @progbits
