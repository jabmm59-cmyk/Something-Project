/* In-memory file system. Fixed-size tables, no dynamic allocation. */
#include "fs.h"
#include "kstring.h"

typedef struct {
    unsigned char used, is_dir;
    char name[FS_NAME_MAX + 1];
    int parent;
    int block;
    size_t size;
} node_t;

static node_t nodes[FS_MAX_NODES];
static unsigned char block_used[FS_BLOCKS];
static char blocks[FS_BLOCKS][FS_FILE_MAX];

void fs_init(void) {
    memset(nodes, 0, sizeof nodes);
    memset(block_used, 0, sizeof block_used);
    nodes[FS_ROOT].used = 1;
    nodes[FS_ROOT].is_dir = 1;
    nodes[FS_ROOT].parent = FS_ROOT;
    nodes[FS_ROOT].block = -1;
}

const char *fs_strerror(int err) {
    switch (err) {
    case FS_ENOENT: return "No such file or directory";
    case FS_EEXIST: return "File exists";
    case FS_ENOTDIR: return "Not a directory";
    case FS_EISDIR: return "Is a directory";
    case FS_ENOSPC: return "No space left on device";
    case FS_EINVAL: return "Invalid argument";
    case FS_ENOTEMPTY: return "Directory not empty";
    case FS_ENAMETOOLONG: return "File name too long";
    default: return "Unknown error";
    }
}

int fs_valid(int n) { return n >= 0 && n < FS_MAX_NODES && nodes[n].used; }
int fs_is_dir(int n) { return fs_valid(n) && nodes[n].is_dir; }
const char *fs_name(int n) { return fs_valid(n) ? nodes[n].name : ""; }
int fs_parent(int n) { return fs_valid(n) ? nodes[n].parent : FS_ROOT; }
size_t fs_size(int n) { return fs_valid(n) ? nodes[n].size : 0; }
const char *fs_data(int n) {
    if (!fs_valid(n) || nodes[n].block < 0) return "";
    return blocks[nodes[n].block];
}

int fs_free_blocks(void) {
    int free_count = 0;
    for (int i = 0; i < FS_BLOCKS; i++) if (!block_used[i]) free_count++;
    return free_count;
}

static int find_child(int dir, const char *name, size_t len) {
    for (int i = 1; i < FS_MAX_NODES; i++) {
        if (nodes[i].used && nodes[i].parent == dir && ks_len(nodes[i].name) == len &&
            memcmp(nodes[i].name, name, len) == 0)
            return i;
    }
    return -1;
}

static int alloc_node(void) {
    for (int i = 1; i < FS_MAX_NODES; i++) if (!nodes[i].used) return i;
    return FS_ENOSPC;
}

static int name_ok(const char *name) {
    size_t n = ks_len(name);
    if (n == 0) return FS_EINVAL;
    if (n > FS_NAME_MAX) return FS_ENAMETOOLONG;
    if (n == 1 && name[0] == '.') return FS_EINVAL;
    if (n == 2 && name[0] == '.' && name[1] == '.') return FS_EINVAL;
    for (size_t i = 0; i < n; i++) if (name[i] == '/') return FS_EINVAL;
    return 0;
}

static int new_node(int parent, const char *name, int is_dir) {
    int n = alloc_node();
    if (n < 0) return n;
    memset(&nodes[n], 0, sizeof nodes[n]);
    nodes[n].used = 1;
    nodes[n].is_dir = (unsigned char)is_dir;
    nodes[n].parent = parent;
    nodes[n].block = -1;
    ks_copy(nodes[n].name, name, sizeof nodes[n].name);
    return n;
}

int fs_resolve(const char *path, int cwd) {
    if (!fs_valid(cwd)) cwd = FS_ROOT;
    int cur = (path[0] == '/') ? FS_ROOT : cwd;
    const char *p = path;
    while (*p) {
        while (*p == '/') p++;
        if (!*p) break;
        const char *s = p;
        while (*p && *p != '/') p++;
        size_t len = (size_t)(p - s);
        if (!nodes[cur].is_dir) return FS_ENOTDIR;
        if (len == 1 && s[0] == '.') continue;
        if (len == 2 && s[0] == '.' && s[1] == '.') { cur = nodes[cur].parent; continue; }
        int c = find_child(cur, s, len);
        if (c < 0) return FS_ENOENT;
        cur = c;
    }
    return cur;
}

/* Split path into parent directory node and last component. */
static int split(const char *path, int cwd, int *parent, char *name) {
    size_t n = ks_len(path);
    if (n == 0) return FS_EINVAL;
    if (n >= 256) return FS_ENAMETOOLONG;
    char tmp[256];
    memcpy(tmp, path, n + 1);
    while (n > 1 && tmp[n - 1] == '/') tmp[--n] = 0;
    if (n == 1 && tmp[0] == '/') return FS_EINVAL;
    char *slash = NULL;
    for (size_t i = n; i > 0; i--) if (tmp[i - 1] == '/') { slash = &tmp[i - 1]; break; }
    const char *base;
    int dir;
    if (!slash) { base = tmp; dir = fs_valid(cwd) ? cwd : FS_ROOT; }
    else {
        base = slash + 1;
        if (slash == tmp) dir = FS_ROOT;
        else { *slash = 0; dir = fs_resolve(tmp, cwd); if (dir < 0) return dir; }
    }
    if (!nodes[dir].is_dir) return FS_ENOTDIR;
    if (ks_len(base) > FS_NAME_MAX) return FS_ENAMETOOLONG;
    ks_copy(name, base, FS_NAME_MAX + 1);
    *parent = dir;
    return 0;
}

int fs_mkdir(const char *path, int cwd) {
    int parent; char name[FS_NAME_MAX + 1];
    int e = split(path, cwd, &parent, name);
    if (e < 0) return e;
    if ((e = name_ok(name)) < 0) return e;
    if (find_child(parent, name, ks_len(name)) >= 0) return FS_EEXIST;
    e = new_node(parent, name, 1);
    return e < 0 ? e : 0;
}

int fs_mkdir_p(const char *path, int cwd) {
    if (!fs_valid(cwd)) cwd = FS_ROOT;
    int cur = (path[0] == '/') ? FS_ROOT : cwd;
    const char *p = path;
    while (*p) {
        while (*p == '/') p++;
        if (!*p) break;
        const char *s = p;
        while (*p && *p != '/') p++;
        size_t len = (size_t)(p - s);
        if (len == 1 && s[0] == '.') continue;
        if (len == 2 && s[0] == '.' && s[1] == '.') { cur = nodes[cur].parent; continue; }
        if (len > FS_NAME_MAX) return FS_ENAMETOOLONG;
        int c = find_child(cur, s, len);
        if (c < 0) {
            char name[FS_NAME_MAX + 1];
            memcpy(name, s, len); name[len] = 0;
            c = new_node(cur, name, 1);
            if (c < 0) return c;
        } else if (!nodes[c].is_dir) return FS_ENOTDIR;
        cur = c;
    }
    return 0;
}

int fs_create(const char *path, int cwd) {
    int parent; char name[FS_NAME_MAX + 1];
    int e = split(path, cwd, &parent, name);
    if (e < 0) return e;
    if ((e = name_ok(name)) < 0) return e;
    int c = find_child(parent, name, ks_len(name));
    if (c >= 0) return nodes[c].is_dir ? FS_EISDIR : c;
    return new_node(parent, name, 0);
}

static int alloc_block(void) {
    for (int i = 0; i < FS_BLOCKS; i++) if (!block_used[i]) { block_used[i] = 1; return i; }
    return -1;
}

int fs_write(int node, const char *data, size_t len, int append) {
    if (!fs_valid(node)) return FS_ENOENT;
    if (nodes[node].is_dir) return FS_EISDIR;
    size_t base = append ? nodes[node].size : 0;
    size_t total = base + len;
    if (total > FS_FILE_MAX) return FS_ENOSPC;
    if (total == 0) {
        if (nodes[node].block >= 0) { block_used[nodes[node].block] = 0; nodes[node].block = -1; }
        nodes[node].size = 0;
        return 0;
    }
    if (nodes[node].block < 0) {
        int b = alloc_block();
        if (b < 0) return FS_ENOSPC;
        nodes[node].block = b;
    }
    memmove(blocks[nodes[node].block] + base, data, len);
    nodes[node].size = total;
    return 0;
}

static void free_node(int n) {
    if (nodes[n].block >= 0) block_used[nodes[n].block] = 0;
    memset(&nodes[n], 0, sizeof nodes[n]);
    nodes[n].block = -1;
}

static int count_children(int dir) {
    int c = 0;
    for (int i = 1; i < FS_MAX_NODES; i++) if (nodes[i].used && nodes[i].parent == dir) c++;
    return c;
}

int fs_remove_node(int node, int recursive) {
    if (!fs_valid(node) || node == FS_ROOT) return FS_EINVAL;
    if (nodes[node].is_dir && count_children(node) > 0) {
        if (!recursive) return FS_ENOTEMPTY;
        for (int i = 1; i < FS_MAX_NODES; i++)
            if (nodes[i].used && nodes[i].parent == node) fs_remove_node(i, 1);
    }
    free_node(node);
    return 0;
}

int fs_children(int dir, int *out, int max) {
    int n = 0;
    if (!fs_is_dir(dir)) return 0;
    for (int i = 1; i < FS_MAX_NODES && n < max; i++)
        if (nodes[i].used && nodes[i].parent == dir) out[n++] = i;
    for (int i = 1; i < n; i++) {           /* insertion sort by name */
        int v = out[i], j = i - 1;
        while (j >= 0 && ks_cmp(nodes[out[j]].name, nodes[v].name) > 0) { out[j + 1] = out[j]; j--; }
        out[j + 1] = v;
    }
    return n;
}

int fs_path(int node, char *buf, size_t size) {
    if (!fs_valid(node) || size < 2) return FS_EINVAL;
    if (node == FS_ROOT) { ks_copy(buf, "/", size); return 0; }
    int chain[64], depth = 0, n = node;
    while (n != FS_ROOT) {
        if (depth == 64) return FS_ENAMETOOLONG;
        chain[depth++] = n;
        n = nodes[n].parent;
    }
    size_t pos = 0;
    for (int i = depth - 1; i >= 0; i--) {
        size_t len = ks_len(nodes[chain[i]].name);
        if (pos + 1 + len + 1 > size) return FS_ENAMETOOLONG;
        buf[pos++] = '/';
        memcpy(buf + pos, nodes[chain[i]].name, len);
        pos += len;
    }
    buf[pos] = 0;
    return 0;
}

/* Destination for copy/move: an existing directory (keep the source name) or a new name. */
static int dest_for(int src, const char *dst, int cwd, int *parent, char *name) {
    int d = fs_resolve(dst, cwd);
    if (d >= 0 && nodes[d].is_dir) {
        *parent = d;
        ks_copy(name, nodes[src].name, FS_NAME_MAX + 1);
        return 0;
    }
    int e = split(dst, cwd, parent, name);
    if (e < 0) return e;
    return name_ok(name);
}

int fs_copy(const char *src, const char *dst, int cwd) {
    int s = fs_resolve(src, cwd);
    if (s < 0) return s;
    if (nodes[s].is_dir) return FS_EISDIR;
    int parent; char name[FS_NAME_MAX + 1];
    int e = dest_for(s, dst, cwd, &parent, name);
    if (e < 0) return e;
    int c = find_child(parent, name, ks_len(name));
    if (c == s) return FS_EINVAL;
    if (c >= 0 && nodes[c].is_dir) return FS_EISDIR;
    if (c < 0) { c = new_node(parent, name, 0); if (c < 0) return c; }
    return fs_write(c, fs_data(s), nodes[s].size, 0);
}

int fs_move(const char *src, const char *dst, int cwd) {
    int s = fs_resolve(src, cwd);
    if (s < 0) return s;
    if (s == FS_ROOT) return FS_EINVAL;
    int parent; char name[FS_NAME_MAX + 1];
    int e = dest_for(s, dst, cwd, &parent, name);
    if (e < 0) return e;
    for (int a = parent; ; a = nodes[a].parent) {       /* cannot move into itself */
        if (a == s) return FS_EINVAL;
        if (a == FS_ROOT) break;
    }
    int c = find_child(parent, name, ks_len(name));
    if (c == s) return 0;
    if (c >= 0) return FS_EEXIST;
    nodes[s].parent = parent;
    ks_copy(nodes[s].name, name, sizeof nodes[s].name);
    return 0;
}

int fs_rename_node(int node, const char *newname) {
    if (!fs_valid(node) || node == FS_ROOT) return FS_EINVAL;
    int e = name_ok(newname);
    if (e < 0) return e;
    int c = find_child(nodes[node].parent, newname, ks_len(newname));
    if (c == node) return 0;
    if (c >= 0) return FS_EEXIST;
    ks_copy(nodes[node].name, newname, sizeof nodes[node].name);
    return 0;
}
