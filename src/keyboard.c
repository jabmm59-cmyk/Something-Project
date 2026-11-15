/* Kernel-only: polls the PS/2 controller (no interrupts yet). */
#include "keymap.h"
#include "io.h"

static kbd_state state;

int kbd_getkey(void) {
    for (;;) {
        uint8_t status = inb(0x64);
        if (!(status & 1)) { __asm__ volatile("pause"); continue; }
        uint8_t sc = inb(0x60);
        if (status & 0x20) continue;            /* byte from the mouse port: ignore */
        int k = kbd_feed(&state, sc);
        if (k) return k;
    }
}
