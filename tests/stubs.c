/* Host-side stand-ins for hardware-dependent pieces used by the shell. */
#include <string.h>
#include "console.h"
#include "editor.h"
#include "fm.h"
#include "plat.h"

char out_log[65536];
size_t out_len;
int editor_calls, fm_calls, clear_calls, reboot_calls, halt_calls;
char editor_arg[128];

void out_reset(void) { out_len = 0; out_log[0] = 0; }
void kputc(char c) { if (out_len + 1 < sizeof out_log) { out_log[out_len++] = c; out_log[out_len] = 0; } }
void kputs(const char *s) { while (*s) kputc(*s++); }
void con_clear(void) { clear_calls++; }
int editor_run(const char *path) { editor_calls++; strncpy(editor_arg, path, sizeof editor_arg - 1); return 0; }
void fm_run(void) { fm_calls++; }
void plat_reboot(void) { reboot_calls++; }
void plat_halt(void) { halt_calls++; }
#include "keymap.h"
int kbd_getkey(void) { return 0; }
