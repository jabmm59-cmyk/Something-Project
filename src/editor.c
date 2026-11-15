/* nano-like full-screen editor. ^O save, ^X exit, ^K cut line, ^U paste, ^G help. */
#include "editor.h"
#include "editor_core.h"
#include "console.h"
#include "keymap.h"
#include "shell.h"
#include "ui.h"
#include "fs.h"
#include "kstring.h"

#define TEXT_ROWS 22          /* screen rows 1..22 */

static ebuf eb;
static char clip[256];
static int cliplen;

static void draw(const char *name, int top) {
    con_fill_row(0, ATTR_REV);
    con_text_at(1, 0, "Something nano", ATTR_REV);
    con_text_at(20, 0, name[0] ? name : "(unnamed)", ATTR_REV);
    if (eb.modified) con_text_at(CON_COLS - 11, 0, "[Modified]", ATTR_REV);

    int off = 0, l = 0, ended = 0;
    while (l < top && off < eb.len) { if (eb.data[off] == '\n') l++; off++; }
    for (int r = 0; r < TEXT_ROWS; r++) {
        int y = 1 + r, x = 0;
        if (!ended) {
            while (off < eb.len && eb.data[off] != '\n') {
                if (x < CON_COLS) con_put_at(x++, y, eb.data[off], ATTR_NORMAL);
                off++;
            }
            if (off < eb.len) off++; else ended = 1;
        }
        for (; x < CON_COLS; x++) con_put_at(x, y, ' ', ATTR_NORMAL);
    }
    con_fill_row(UI_HELP_ROW, ATTR_REV);
    con_text_at(1, UI_HELP_ROW, "^O Write Out   ^X Exit   ^K Cut Line   ^U Paste   ^G Help", ATTR_REV);
}

static int save(char *name, int namemax) {
    if (!name[0]) {
        if (!ui_prompt("File Name to Write: ", name, namemax)) { ui_status("Cancelled"); return -1; }
    }
    int node = fs_resolve(name, sh_cwd);
    if (node >= 0 && fs_is_dir(node)) { ui_status("Error: that is a directory"); name[0] = 0; return -1; }
    if (node < 0) node = fs_create(name, sh_cwd);
    if (node < 0) { ui_status(fs_strerror(node)); name[0] = 0; return -1; }
    int e = fs_write(node, eb.data, (size_t)eb.len, 0);
    if (e < 0) { ui_status(fs_strerror(e)); return -1; }
    eb.modified = 0;
    ui_status("Wrote file");
    return 0;
}

int editor_run(const char *path) {
    char name[64];
    ks_copy(name, path, sizeof name);
    int node = name[0] ? fs_resolve(name, sh_cwd) : -1;
    if (node >= 0 && fs_is_dir(node)) return FS_EISDIR;
    if (node >= 0) eb_init(&eb, fs_data(node), (int)fs_size(node)); else eb_init(&eb, "", 0);

    int top = 0, warned = 0;
    con_clear();
    con_cursor_enable(1);
    ui_status(node >= 0 ? "" : (name[0] ? "New file" : ""));
    for (;;) {
        int line = eb_line(&eb);
        if (line < top) top = line;
        if (line >= top + TEXT_ROWS) top = line - TEXT_ROWS + 1;
        draw(name, top);
        int col = eb_col(&eb);
        con_set_cursor(col < CON_COLS ? col : CON_COLS - 1, 1 + line - top);

        int k = kbd_getkey();
        if (k != 24) warned = 0;
        if (k >= 32 && k < 127) { if (eb_insert(&eb, (char)k) < 0) ui_status("File full (4096 bytes)"); }
        else if (k == '\n' || k == 13) { if (eb_insert(&eb, '\n') < 0) ui_status("File full (4096 bytes)"); }
        else if (k == 9) { for (int i = 0; i < 4; i++) if (eb_insert(&eb, ' ') < 0) break; }
        else if (k == 8) eb_backspace(&eb);
        else if (k == KEY_DEL) eb_delete(&eb);
        else if (k == KEY_LEFT) eb_left(&eb);
        else if (k == KEY_RIGHT) eb_right(&eb);
        else if (k == KEY_UP) eb_up(&eb);
        else if (k == KEY_DOWN) eb_down(&eb);
        else if (k == KEY_HOME || k == 1) eb_home(&eb);
        else if (k == KEY_END || k == 5) eb_end(&eb);
        else if (k == KEY_PGUP) { for (int i = 0; i < TEXT_ROWS - 2; i++) eb_up(&eb); }
        else if (k == KEY_PGDN) { for (int i = 0; i < TEXT_ROWS - 2; i++) eb_down(&eb); }
        else if (k == 11) { int n = eb_cut_line(&eb, clip, (int)sizeof clip); if (n) cliplen = n; }
        else if (k == 21) eb_paste(&eb, clip, cliplen);
        else if (k == 15) save(name, (int)sizeof name);
        else if (k == 7) ui_status("^O save  ^X exit  ^K cut line  ^U paste  ^A/^E line start/end  arrows/PgUp/PgDn move");
        else if (k == 24) {
            if (eb.modified && !warned) { warned = 1; ui_status("Unsaved changes! ^O to save, ^X again to discard"); }
            else break;
        }
    }
    return 0;
}
