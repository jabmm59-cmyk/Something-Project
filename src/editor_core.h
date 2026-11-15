#ifndef EDITOR_CORE_H
#define EDITOR_CORE_H
#include "fs.h"

typedef struct { char data[FS_FILE_MAX]; int len, cur, modified; } ebuf;

void eb_init(ebuf *e, const char *data, int len);
int eb_insert(ebuf *e, char c);              /* 0 ok, -1 full */
void eb_backspace(ebuf *e);
void eb_delete(ebuf *e);
void eb_left(ebuf *e);
void eb_right(ebuf *e);
void eb_up(ebuf *e);
void eb_down(ebuf *e);
void eb_home(ebuf *e);
void eb_end(ebuf *e);
int eb_line(const ebuf *e);                  /* cursor line, 0-based */
int eb_col(const ebuf *e);                   /* cursor column, 0-based */
int eb_cut_line(ebuf *e, char *clip, int max);   /* returns bytes cut */
void eb_paste(ebuf *e, const char *clip, int n);
#endif
