bits 16
org 0x7C00

start:
    cli

    mov [boot_drive], dl

    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00

    ; Carregar kernel para 0x100000
    mov si, disk_packet
    mov word [disk_packet + 2], 128
    mov word [disk_packet + 4], 0x0000
    mov word [disk_packet + 6], 0x0010

    mov eax, 1
    mov dword [disk_packet + 8], eax
    mov dword [disk_packet + 12], 0

    mov dl, [boot_drive]
    mov ah, 0x42
    int 0x13

    jc disk_error

    ; Entrar no modo protegido
    cli

    lgdt [gdt_descriptor]

    mov eax, cr0
    or eax, 1
    mov cr0, eax

    jmp 0x08:protected_mode


disk_error:
    mov si, error_message

.print:
    lodsb

    cmp al, 0
    je halt

    mov ah, 0x0E
    int 0x10

    jmp .print


halt:
    cli
    hlt
    jmp halt


bits 32

protected_mode:
    mov ax, 0x10

    mov ds, ax
    mov es, ax
    mov ss, ax

    mov esp, 0x90000

    ; Kernel está em 0x100000
    jmp 0x100000


boot_drive db 0

error_message db "Something Project: Disk error!", 0


; Disk Address Packet
disk_packet:
    db 0x10
    db 0
    dw 128
    dw 0
    dw 0x0010
    dq 1


; GDT
gdt_start:

    dq 0

gdt_code:
    dw 0xFFFF
    dw 0
    db 0
    db 10011010b
    db 11001111b
    db 0

gdt_data:
    dw 0xFFFF
    dw 0
    db 0
    db 10010010b
    db 11001111b
    db 0

gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start


times 510 - ($ - $$) db 0
dw 0xAA55
