#ifndef KSTRING_H
#define KSTRING_H
#include <stddef.h>
#ifdef KERNEL
void *memcpy(void *d, const void *s, size_t n);
void *memmove(void *d, const void *s, size_t n);
void *memset(void *d, int c, size_t n);
int memcmp(const void *a, const void *b, size_t n);
#else
#include <string.h>
#endif

static inline size_t ks_len(const char *s) { size_t n = 0; while (s[n]) n++; return n; }
static inline int ks_cmp(const char *a, const char *b) {
    while (*a && *a == *b) { a++; b++; }
    return (unsigned char)*a - (unsigned char)*b;
}
static inline void ks_copy(char *d, const char *s, size_t max) {
    size_t i = 0;
    if (max == 0) return;
    while (i + 1 < max && s[i]) { d[i] = s[i]; i++; }
    d[i] = 0;
}
#endif
