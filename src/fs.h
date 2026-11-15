#ifndef FS_H
#define FS_H
#include <stddef.h>

#define FS_MAX_NODES 256
#define FS_BLOCKS 192
#define FS_NAME_MAX 31
#define FS_FILE_MAX 4096
#define FS_ROOT 0

enum { FS_OK = 0, FS_ENOENT = -1, FS_EEXIST = -2, FS_ENOTDIR = -3, FS_EISDIR = -4,
       FS_ENOSPC = -5, FS_EINVAL = -6, FS_ENOTEMPTY = -7, FS_ENAMETOOLONG = -8 };

void fs_init(void);
const char *fs_strerror(int err);

int fs_valid(int node);
int fs_is_dir(int node);
const char *fs_name(int node);
int fs_parent(int node);
size_t fs_size(int node);
const char *fs_data(int node);

int fs_resolve(const char *path, int cwd);          /* node or negative error */
int fs_mkdir(const char *path, int cwd);
int fs_mkdir_p(const char *path, int cwd);
int fs_create(const char *path, int cwd);           /* existing file or new file: node; dir: FS_EISDIR */
int fs_write(int node, const char *data, size_t len, int append);
int fs_remove_node(int node, int recursive);
int fs_children(int dir, int *out, int max);        /* sorted by name */
int fs_path(int node, char *buf, size_t size);
int fs_copy(const char *src, const char *dst, int cwd);
int fs_move(const char *src, const char *dst, int cwd);
int fs_rename_node(int node, const char *newname);
int fs_free_blocks(void);
#endif
