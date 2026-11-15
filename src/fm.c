/* Full-screen file manager. */
#include "fm.h"
#include "console.h"
#include "keymap.h"
#include "shell.h"
#include "editor.h"
#include "ui.h"
#include "fs.h"
#include "kstring.h"

#define LIST_TOP 2
#define LIST_ROWS 21          /* rows 2..22 */

static void put_num(int x, int y, unsigned long v, unsigned char attr) {
    char tmp[24]; int n = 0;
    if (v == 0) tmp[n++] = '0';
    while (v) { tmp[n++] = (char)('0' + v % 10); v /= 10; }
    for (int i = 0; i < n; i++) con_put_at(x + i, y, tmp[n - 1 - i], attr);
}

/* list[i] = node, or -1 for the ".." entry */
static int build_list(int *list) {
    int count = 0;
    if (sh_cwd != FS_ROOT) list[count++] = -1;
    count += fs_children(sh_cwd, list + count, FS_MAX_NODES);
    return count;
}

static void draw(const int *list, int count, int sel, int scroll, const char *msg) {
    char path[128];
    con_clear();
    con_cursor_enable(0);
    con_fill_row(0, ATTR_REV);
    con_text_at(1, 0, "File Manager", ATTR_REV);
    if (fs_path(sh_cwd, path, sizeof path) == 0) con_text_at(16, 0, path, ATTR_REV);
    for (int r = 0; r < LIST_ROWS && scroll + r < count; r++) {
        int i = scroll + r, y = LIST_TOP + r;
        unsigned char attr = (i == sel) ? ATTR_REV : ATTR_NORMAL;
        if (i == sel) con_fill_row(y, attr);
        if (list[i] < 0) { con_text_at(2, y, "../", attr); continue; }
        con_text_at(2, y, fs_name(list[i]), attr);
        if (fs_is_dir(list[i])) con_text_at(2 + (int)ks_len(fs_name(list[i])), y, "/", attr);
        else { put_num(60, y, fs_size(list[i]), attr); con_text_at(70, y, "bytes", attr); }
    }
    if (count == 0) con_text_at(2, LIST_TOP, "(empty directory)", ATTR_NORMAL);
    if (msg[0]) ui_status(msg);
    con_fill_row(UI_HELP_ROW, ATTR_REV);
    con_text_at(1, UI_HELP_ROW, "Enter Open  Bksp Up  n New file  m New dir  r Rename  Del Delete  q Quit", ATTR_REV);
}

void fm_run(void) {
    int list[FS_MAX_NODES + 1];
    int sel = 0, scroll = 0;
    const char *msg = "";
    char name[FS_NAME_MAX + 1], path[128];
    for (;;) {
        int count = build_list(list);
        if (sel >= count) sel = count ? count - 1 : 0;
        if (sel < scroll) scroll = sel;
        if (sel >= scroll + LIST_ROWS) scroll = sel - LIST_ROWS + 1;
        draw(list, count, sel, scroll, msg);
        msg = "";
        int k = kbd_getkey();
        int cur = count ? list[sel] : -2;

        if (k == KEY_UP) { if (sel > 0) sel--; }
        else if (k == KEY_DOWN) { if (sel + 1 < count) sel++; }
        else if (k == 'q' || k == 27) break;
        else if (k == '\n' && count) {
            if (cur == -1) { sh_cwd = fs_parent(sh_cwd); sel = scroll = 0; }
            else if (fs_is_dir(cur)) { sh_cwd = cur; sel = scroll = 0; }
            else if (fs_path(cur, path, sizeof path) == 0) { editor_run(path); }
        }
        else if ((k == 8 || k == KEY_LEFT) && sh_cwd != FS_ROOT) { sh_cwd = fs_parent(sh_cwd); sel = scroll = 0; }
        else if (k == 'n' || k == 'm') {
            if (ui_prompt(k == 'n' ? "New file name: " : "New directory name: ", name, sizeof name)) {
                int e = (fs_resolve(name, sh_cwd) >= 0) ? FS_EEXIST
                      : (k == 'n' ? fs_create(name, sh_cwd) : fs_mkdir(name, sh_cwd));
                msg = e < 0 ? fs_strerror(e) : "Created";
            }
        }
        else if (k == 'r' && count && cur >= 0) {
            if (ui_prompt("New name: ", name, sizeof name)) {
                int e = fs_rename_node(cur, name);
                msg = e < 0 ? fs_strerror(e) : "Renamed";
            }
        }
        else if ((k == KEY_DEL || k == 'x') && count && cur >= 0) {
            ui_status(fs_is_dir(cur) ? "Delete this directory and everything in it? (y/n)" : "Delete this file? (y/n)");
            if (kbd_getkey() == 'y') {
                int e = fs_remove_node(cur, 1);
                msg = e < 0 ? fs_strerror(e) : "Deleted";
            } else msg = "Cancelled";
        }
    }
    con_cursor_enable(1);
}
