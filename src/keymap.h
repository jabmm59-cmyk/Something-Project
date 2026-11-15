#ifndef KEYMAP_H
#define KEYMAP_H
enum { KEY_UP = 0x100, KEY_DOWN, KEY_LEFT, KEY_RIGHT, KEY_HOME, KEY_END, KEY_DEL, KEY_PGUP, KEY_PGDN };

typedef struct { int shift, ctrl, caps, ext; } kbd_state;

void kbd_state_init(kbd_state *s);
/* Feed one PS/2 set-1 scancode; returns a key code (ASCII, Ctrl+letter = 1..26, or KEY_*) or 0. */
int kbd_feed(kbd_state *s, unsigned char scancode);
/* Blocking read (kernel only, polls the controller). */
int kbd_getkey(void);
#endif
