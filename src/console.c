/* VGA text mode console (80x25 at 0xB8000). */
#include <stdint.h>
#include "console.h"
#include "io.h"

#define VGA ((volatile uint16_t *)0xB8000)

static int cx, cy;

static uint16_t cell(char c, unsigned char attr) { return (uint16_t)((attr << 8) | (uint8_t)c); }

void con_set_cursor(int x, int y) {
    uint16_t pos = (uint16_t)(y * CON_COLS + x);
    outb(0x3D4, 14); outb(0x3D5, (uint8_t)(pos >> 8));
    outb(0x3D4, 15); outb(0x3D5, (uint8_t)(pos & 0xFF));
}

void con_cursor_enable(int on) {
    if (on) {
        outb(0x3D4, 0x0A); outb(0x3D5, (uint8_t)((inb(0x3D5) & 0xC0) | 14));
        outb(0x3D4, 0x0B); outb(0x3D5, (uint8_t)((inb(0x3D5) & 0xE0) | 15));
    } else {
        outb(0x3D4, 0x0A); outb(0x3D5, 0x20);
    }
}

void con_clear(void) {
    for (int i = 0; i < CON_COLS * CON_ROWS; i++) VGA[i] = cell(' ', ATTR_NORMAL);
    cx = cy = 0;
    con_set_cursor(0, 0);
}

static void scroll(void) {
    for (int i = 0; i < CON_COLS * (CON_ROWS - 1); i++) VGA[i] = VGA[i + CON_COLS];
    for (int i = CON_COLS * (CON_ROWS - 1); i < CON_COLS * CON_ROWS; i++) VGA[i] = cell(' ', ATTR_NORMAL);
    cy = CON_ROWS - 1;
}

void kputc(char c) {
    if (c == '\n') { cx = 0; cy++; }
    else if (c == '\b') {
        if (cx > 0) cx--;
        else if (cy > 0) { cy--; cx = CON_COLS - 1; }
        VGA[cy * CON_COLS + cx] = cell(' ', ATTR_NORMAL);
    } else if (c == '\t') {
        int n = 8 - (cx % 8);
        while (n--) kputc(' ');
        return;
    } else {
        VGA[cy * CON_COLS + cx] = cell(c, ATTR_NORMAL);
        if (++cx >= CON_COLS) { cx = 0; cy++; }
    }
    if (cy >= CON_ROWS) scroll();
    con_set_cursor(cx, cy);
}

void kputs(const char *s) { while (*s) kputc(*s++); }

void con_put_at(int x, int y, char c, unsigned char attr) {
    if (x < 0 || x >= CON_COLS || y < 0 || y >= CON_ROWS) return;
    VGA[y * CON_COLS + x] = cell(c, attr);
}

void con_text_at(int x, int y, const char *s, unsigned char attr) {
    while (*s && x < CON_COLS) con_put_at(x++, y, *s++, attr);
}

void con_fill_row(int y, unsigned char attr) {
    for (int x = 0; x < CON_COLS; x++) con_put_at(x, y, ' ', attr);
}
