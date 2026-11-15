/* Text buffer operations for the editor. Pure logic, testable on the host. */
#include "editor_core.h"
#include "kstring.h"

void eb_init(ebuf *e, const char *data, int len) {
    if (len > FS_FILE_MAX) len = FS_FILE_MAX;
    memcpy(e->data, data, (size_t)len);
    e->len = len; e->cur = 0; e->modified = 0;
}

static int line_start(const ebuf *e, int pos) {
    while (pos > 0 && e->data[pos - 1] != '\n') pos--;
    return pos;
}
static int line_end(const ebuf *e, int pos) {
    while (pos < e->len && e->data[pos] != '\n') pos++;
    return pos;
}

int eb_insert(ebuf *e, char c) {
    if (e->len >= FS_FILE_MAX) return -1;
    memmove(e->data + e->cur + 1, e->data + e->cur, (size_t)(e->len - e->cur));
    e->data[e->cur++] = c;
    e->len++;
    e->modified = 1;
    return 0;
}

void eb_backspace(ebuf *e) {
    if (e->cur == 0) return;
    memmove(e->data + e->cur - 1, e->data + e->cur, (size_t)(e->len - e->cur));
    e->cur--; e->len--; e->modified = 1;
}

void eb_delete(ebuf *e) {
    if (e->cur >= e->len) return;
    memmove(e->data + e->cur, e->data + e->cur + 1, (size_t)(e->len - e->cur - 1));
    e->len--; e->modified = 1;
}

void eb_left(ebuf *e) { if (e->cur > 0) e->cur--; }
void eb_right(ebuf *e) { if (e->cur < e->len) e->cur++; }
void eb_home(ebuf *e) { e->cur = line_start(e, e->cur); }
void eb_end(ebuf *e) { e->cur = line_end(e, e->cur); }

void eb_up(ebuf *e) {
    int ls = line_start(e, e->cur);
    if (ls == 0) return;
    int col = e->cur - ls;
    int prev_end = ls - 1;
    int prev_start = line_start(e, prev_end);
    int plen = prev_end - prev_start;
    e->cur = prev_start + (col < plen ? col : plen);
}

void eb_down(ebuf *e) {
    int le = line_end(e, e->cur);
    if (le >= e->len) return;
    int col = e->cur - line_start(e, e->cur);
    int next_start = le + 1;
    int next_end = line_end(e, next_start);
    int nlen = next_end - next_start;
    e->cur = next_start + (col < nlen ? col : nlen);
}

int eb_line(const ebuf *e) {
    int n = 0;
    for (int i = 0; i < e->cur; i++) if (e->data[i] == '\n') n++;
    return n;
}
int eb_col(const ebuf *e) { return e->cur - line_start(e, e->cur); }

int eb_cut_line(ebuf *e, char *clip, int max) {
    int ls = line_start(e, e->cur);
    int le = line_end(e, e->cur);
    int end = (le < e->len) ? le + 1 : le;
    int n = end - ls;
    if (n <= 0) return 0;
    int keep = n < max ? n : max;
    memcpy(clip, e->data + ls, (size_t)keep);
    memmove(e->data + ls, e->data + end, (size_t)(e->len - end));
    e->len -= n;
    e->cur = ls;
    e->modified = 1;
    return keep;
}

void eb_paste(ebuf *e, const char *clip, int n) {
    for (int i = 0; i < n; i++) if (eb_insert(e, clip[i]) < 0) break;
}
