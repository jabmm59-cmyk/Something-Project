/* PS/2 scancode set 1 decoder, US layout. Pure logic, testable on the host. */
#include "keymap.h"

#define NKEYS 58
static const char normal[NKEYS] =
    "\0" "\033" "1234567890" "-=" "\b\t" "qwertyuiop" "[]" "\n" "\0"
    "asdfghjkl" ";'`" "\0" "\\" "zxcvbnm" ",./" "\0" "*" "\0" " ";
static const char shifted[NKEYS] =
    "\0" "\033" "!@#$%^&*()" "_+" "\b\t" "QWERTYUIOP" "{}" "\n" "\0"
    "ASDFGHJKL" ":\"~" "\0" "|" "ZXCVBNM" "<>?" "\0" "*" "\0" " ";

void kbd_state_init(kbd_state *s) { s->shift = s->ctrl = s->caps = s->ext = 0; }

int kbd_feed(kbd_state *s, unsigned char sc) {
    if (sc == 0xE0) { s->ext = 1; return 0; }
    int ext = s->ext;
    s->ext = 0;
    int release = sc & 0x80;
    sc &= 0x7F;
    if (sc == 0x2A || sc == 0x36) { s->shift = !release; return 0; }
    if (sc == 0x1D) { s->ctrl = !release; return 0; }
    if (release) return 0;
    if (ext) {
        switch (sc) {
        case 0x48: return KEY_UP;
        case 0x50: return KEY_DOWN;
        case 0x4B: return KEY_LEFT;
        case 0x4D: return KEY_RIGHT;
        case 0x47: return KEY_HOME;
        case 0x4F: return KEY_END;
        case 0x53: return KEY_DEL;
        case 0x49: return KEY_PGUP;
        case 0x51: return KEY_PGDN;
        case 0x1C: return '\n';
        default: return 0;
        }
    }
    if (sc == 0x3A) { s->caps = !s->caps; return 0; }
    if (sc >= NKEYS) return 0;
    char base = normal[sc];
    if (!base) return 0;
    int letter = base >= 'a' && base <= 'z';
    if (letter) {
        if (s->ctrl) return base & 0x1F;
        return (s->shift ^ s->caps) ? base - 32 : base;
    }
    return s->shift ? shifted[sc] : base;
}
