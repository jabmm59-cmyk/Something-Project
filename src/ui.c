#include "ui.h"
#include "console.h"
#include "keymap.h"
#include "kstring.h"

void ui_status(const char *msg) {
    con_fill_row(UI_STATUS_ROW, ATTR_NORMAL);
    con_text_at(0, UI_STATUS_ROW, msg, ATTR_NORMAL);
}

int ui_prompt(const char *label, char *out, int max) {
    int n = 0;
    out[0] = 0;
    con_cursor_enable(1);
    for (;;) {
        con_fill_row(UI_STATUS_ROW, ATTR_NORMAL);
        con_text_at(0, UI_STATUS_ROW, label, ATTR_NORMAL);
        int lx = (int)ks_len(label);
        con_text_at(lx, UI_STATUS_ROW, out, ATTR_NORMAL);
        con_set_cursor(lx + n < CON_COLS ? lx + n : CON_COLS - 1, UI_STATUS_ROW);
        int k = kbd_getkey();
        if (k == '\n') { out[n] = 0; return n > 0; }
        if (k == 27) return 0;
        if (k == 8) { if (n > 0) out[--n] = 0; }
        else if (k >= 32 && k < 127 && k != '/' && n < max - 1) { out[n++] = (char)k; out[n] = 0; }
    }
}
