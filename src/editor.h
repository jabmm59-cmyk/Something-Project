#ifndef EDITOR_H
#define EDITOR_H
/* Full-screen editor. path may be "" for an unnamed buffer. Returns 0 or a negative FS_* error. */
int editor_run(const char *path);
#endif
