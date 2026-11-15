ENTRY(_start)
SECTIONS {
    . = 1M;
    .boot   : { KEEP(*(.multiboot2)) }
    .text   : { *(.text .text.*) }
    .rodata : { *(.rodata .rodata.*) }
    .data   : { *(.data .data.*) }
    .bss    : { *(COMMON) *(.bss .bss.*) }
    /DISCARD/ : { *(.comment) *(.eh_frame) *(.note.gnu.property) }
}
