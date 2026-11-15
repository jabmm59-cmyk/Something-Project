#ifndef CONSOLE_H
#define CONSOLE_H
#define CON_COLS 80
#define CON_ROWS 25
#define ATTR_NORMAL 0x07
#define ATTR_REV 0x70

void kputc(char c);
void kputs(const char *s);
void con_clear(void);
void con_put_at(int x, int y, char c, unsigned char attr);
void con_text_at(int x, int y, const char *s, unsigned char attr);
void con_fill_row(int y, unsigned char attr);
void con_set_cursor(int x, int y);
void con_cursor_enable(int on);
#endif
