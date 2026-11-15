/* Command interpreter: cd, ls, mkdir, ... plus > and >> redirection. */
#include "shell.h"
#include "console.h"
#include "fs.h"
#include "editor.h"
#include "fm.h"
#include "plat.h"
#include "keymap.h"
#include "kstring.h"

#define SH_LINE_MAX 256
#define SH_MAX_ARGS 16

int sh_cwd = FS_ROOT;

static char outbuf[FS_FILE_MAX];
static size_t outlen;
static int redirecting;
static char last_out = '\n';

/* ---- output (stdout may be redirected to a file; errors always go to the console) ---- */
static void sh_putc(char c) {
    if (redirecting) { if (outlen < sizeof outbuf) outbuf[outlen++] = c; }
    else { kputc(c); last_out = c; }
}
static void sh_puts(const char *s) { while (*s) sh_putc(*s++); }
static void eputs(const char *s) { while (*s) { kputc(*s); last_out = *s; s++; } }
static void err2(const char *cmd, const char *arg, int e) {
    eputs(cmd); eputs(": "); eputs(arg); eputs(": "); eputs(fs_strerror(e));
    eputs("\n");
}
static void sh_putnum(unsigned long v, int width) {
    char tmp[24]; int n = 0;
    if (v == 0) tmp[n++] = '0';
    while (v) { tmp[n++] = (char)('0' + v % 10); v /= 10; }
    for (int i = n; i < width; i++) sh_putc(' ');
    while (n) sh_putc(tmp[--n]);
}

/* ---- tokenizer: whitespace separated words, quotes, and unquoted > / >> operators ---- */
struct tok { const char *s; int op; };

static int tokenize(const char *line, char *buf, struct tok *t, int max) {
    int n = 0;
    char *w = buf;
    const char *r = line;
    for (;;) {
        while (*r == ' ' || *r == '\t') r++;
        if (!*r) break;
        if (n >= max) return -1;
        if (*r == '>') {
            if (r[1] == '>') { t[n].s = ">>"; r += 2; } else { t[n].s = ">"; r++; }
            t[n].op = 1; n++;
            continue;
        }
        t[n].s = w; t[n].op = 0;
        while (*r && *r != ' ' && *r != '\t' && *r != '>') {
            if (*r == '"' || *r == '\'') {
                char q = *r++;
                while (*r && *r != q) *w++ = *r++;
                if (!*r) return -2;
                r++;
            } else *w++ = *r++;
        }
        *w++ = 0;
        n++;
    }
    return n;
}

/* ---- commands ---- */
typedef int (*cmd_fn)(int argc, const char **argv);

static int cmd_pwd(int argc, const char **argv) {
    (void)argc; (void)argv;
    char p[128];
    if (fs_path(sh_cwd, p, sizeof p) < 0) { eputs("pwd: path too long\n"); return 1; }
    sh_puts(p); sh_putc('\n');
    return 0;
}

static int cmd_cd(int argc, const char **argv) {
    const char *target = argc > 1 ? argv[1] : SH_HOME;
    int n = fs_resolve(target, sh_cwd);
    if (n < 0) { err2("cd", target, n); return 1; }
    if (!fs_is_dir(n)) { err2("cd", target, FS_ENOTDIR); return 1; }
    sh_cwd = n;
    return 0;
}

static void ls_long(int node, const char *name) {
    sh_putc(fs_is_dir(node) ? 'd' : '-');
    sh_putc(' ');
    sh_putnum(fs_is_dir(node) ? 0 : fs_size(node), 6);
    sh_puts("  ");
    sh_puts(name);
    if (fs_is_dir(node)) sh_putc('/');
    sh_putc('\n');
}

static int cmd_ls(int argc, const char **argv) {
    int longfmt = 0;
    const char *target = NULL;
    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-' && argv[i][1]) {
            for (const char *f = argv[i] + 1; *f; f++) {
                if (*f == 'l') longfmt = 1;
                else { eputs("ls: invalid option\n"); return 1; }
            }
        } else target = argv[i];
    }
    int node = target ? fs_resolve(target, sh_cwd) : sh_cwd;
    if (node < 0) { err2("ls", target, node); return 1; }
    if (!fs_is_dir(node)) {
        if (longfmt) ls_long(node, target); else { sh_puts(target); sh_putc('\n'); }
        return 0;
    }
    int items[FS_MAX_NODES];
    int n = fs_children(node, items, FS_MAX_NODES);
    int col = 0;
    for (int i = 0; i < n; i++) {
        const char *name = fs_name(items[i]);
        if (longfmt) { ls_long(items[i], name); continue; }
        int len = (int)ks_len(name) + (fs_is_dir(items[i]) ? 1 : 0);
        if (col > 0 && col + len + 2 > CON_COLS) { sh_putc('\n'); col = 0; }
        sh_puts(name);
        if (fs_is_dir(items[i])) sh_putc('/');
        sh_puts("  ");
        col += len + 2;
    }
    if (col > 0) sh_putc('\n');
    return 0;
}

static int cmd_mkdir(int argc, const char **argv) {
    int parents = 0, rc = 0, files = 0;
    for (int i = 1; i < argc; i++) {
        if (!files && argv[i][0] == '-' && argv[i][1] == 'p' && !argv[i][2]) { parents = 1; continue; }
        files = 1;
        int e = parents ? fs_mkdir_p(argv[i], sh_cwd) : fs_mkdir(argv[i], sh_cwd);
        if (e < 0) { err2("mkdir", argv[i], e); rc = 1; }
    }
    if (!files) { eputs("mkdir: missing operand\n"); return 1; }
    return rc;
}

static int cmd_rmdir(int argc, const char **argv) {
    int rc = 0;
    if (argc < 2) { eputs("rmdir: missing operand\n"); return 1; }
    for (int i = 1; i < argc; i++) {
        int n = fs_resolve(argv[i], sh_cwd);
        int e = n < 0 ? n : (!fs_is_dir(n) ? FS_ENOTDIR : fs_remove_node(n, 0));
        if (e < 0) { err2("rmdir", argv[i], e); rc = 1; }
    }
    return rc;
}

static int cmd_touch(int argc, const char **argv) {
    int rc = 0;
    if (argc < 2) { eputs("touch: missing operand\n"); return 1; }
    for (int i = 1; i < argc; i++) {
        int n = fs_create(argv[i], sh_cwd);
        if (n < 0 && n != FS_EISDIR) { err2("touch", argv[i], n); rc = 1; }
    }
    return rc;
}

static int cmd_rm(int argc, const char **argv) {
    int rec = 0, force = 0, rc = 0, files = 0;
    for (int i = 1; i < argc; i++) {
        if (!files && argv[i][0] == '-' && argv[i][1]) {
            for (const char *f = argv[i] + 1; *f; f++) {
                if (*f == 'r' || *f == 'R') rec = 1;
                else if (*f == 'f') force = 1;
                else { eputs("rm: invalid option\n"); return 1; }
            }
            continue;
        }
        files = 1;
        int n = fs_resolve(argv[i], sh_cwd);
        if (n < 0) { if (!force) { err2("rm", argv[i], n); rc = 1; } continue; }
        if (fs_is_dir(n) && !rec) { err2("rm", argv[i], FS_EISDIR); rc = 1; continue; }
        int e = fs_remove_node(n, rec);
        if (e < 0) { err2("rm", argv[i], e); rc = 1; }
    }
    if (!files && !force) { eputs("rm: missing operand\n"); return 1; }
    return rc;
}

static int cmd_cat(int argc, const char **argv) {
    int rc = 0;
    if (argc < 2) { eputs("cat: missing file operand\n"); return 1; }
    for (int i = 1; i < argc; i++) {
        int n = fs_resolve(argv[i], sh_cwd);
        if (n < 0) { err2("cat", argv[i], n); rc = 1; continue; }
        if (fs_is_dir(n)) { err2("cat", argv[i], FS_EISDIR); rc = 1; continue; }
        const char *d = fs_data(n);
        for (size_t k = 0; k < fs_size(n); k++) sh_putc(d[k]);
    }
    return rc;
}

static int cmd_echo(int argc, const char **argv) {
    int i = 1, newline = 1;
    if (argc > 1 && argv[1][0] == '-' && argv[1][1] == 'n' && !argv[1][2]) { newline = 0; i = 2; }
    for (int first = 1; i < argc; i++, first = 0) {
        if (!first) sh_putc(' ');
        sh_puts(argv[i]);
    }
    if (newline) sh_putc('\n');
    return 0;
}

static int cmd_cp(int argc, const char **argv) {
    if (argc != 3) { eputs("cp: usage: cp SOURCE DEST\n"); return 1; }
    int e = fs_copy(argv[1], argv[2], sh_cwd);
    if (e < 0) { err2("cp", argv[1], e); return 1; }
    return 0;
}

static int cmd_mv(int argc, const char **argv) {
    if (argc != 3) { eputs("mv: usage: mv SOURCE DEST\n"); return 1; }
    int e = fs_move(argv[1], argv[2], sh_cwd);
    if (e < 0) { err2("mv", argv[1], e); return 1; }
    return 0;
}

static int cmd_clear(int argc, const char **argv) {
    (void)argc; (void)argv;
    if (!redirecting) { con_clear(); last_out = '\n'; }
    return 0;
}

static int cmd_nano(int argc, const char **argv) {
    int e = editor_run(argc > 1 ? argv[1] : "");
    con_clear();
    last_out = '\n';
    if (e < 0) { err2("nano", argc > 1 ? argv[1] : "(unnamed)", e); return 1; }
    return 0;
}

static int cmd_fm(int argc, const char **argv) {
    (void)argc; (void)argv;
    fm_run();
    con_clear();
    last_out = '\n';
    return 0;
}

static int cmd_uname(int argc, const char **argv) {
    (void)argc; (void)argv;
    sh_puts("Something Project 0.1.0 x86_64\n");
    return 0;
}

static int cmd_df(int argc, const char **argv) {
    (void)argc; (void)argv;
    int free_b = fs_free_blocks();
    sh_puts("blocks (4096 bytes): used ");
    sh_putnum((unsigned long)(FS_BLOCKS - free_b), 0);
    sh_puts(" / total ");
    sh_putnum(FS_BLOCKS, 0);
    sh_puts("\n");
    return 0;
}

static int cmd_reboot(int argc, const char **argv) { (void)argc; (void)argv; plat_reboot(); return 0; }
static int cmd_halt(int argc, const char **argv) { (void)argc; (void)argv; plat_halt(); return 0; }

static int cmd_help(int argc, const char **argv) {
    (void)argc; (void)argv;
    sh_puts("Commands:\n"
            "  ls [-l] [dir]     list files          cd [dir]          change directory\n"
            "  pwd               current directory   mkdir [-p] dir    create directory\n"
            "  rmdir dir         remove empty dir    touch file        create empty file\n"
            "  rm [-r] [-f] x    remove              cat file          show a file\n"
            "  echo text         print (use > or >> to write to a file)\n"
            "  cp a b            copy a file         mv a b            move or rename\n"
            "  nano [file]       text editor         fm                file manager\n"
            "  df  uname  clear  reboot  halt\n"
            "Editor keys: ^O save, ^X exit, ^K cut line, ^U paste, ^G help.\n");
    return 0;
}

static const struct { const char *name; cmd_fn fn; } commands[] = {
    {"pwd", cmd_pwd}, {"cd", cmd_cd}, {"ls", cmd_ls}, {"mkdir", cmd_mkdir},
    {"rmdir", cmd_rmdir}, {"touch", cmd_touch}, {"rm", cmd_rm}, {"cat", cmd_cat},
    {"echo", cmd_echo}, {"cp", cmd_cp}, {"mv", cmd_mv}, {"clear", cmd_clear},
    {"nano", cmd_nano}, {"edit", cmd_nano}, {"fm", cmd_fm}, {"uname", cmd_uname},
    {"df", cmd_df}, {"reboot", cmd_reboot}, {"halt", cmd_halt}, {"help", cmd_help},
};

void shell_exec(const char *line) {
    static char tokbuf[2 * SH_LINE_MAX + 8];
    struct tok toks[SH_MAX_ARGS];
    const char *argv[SH_MAX_ARGS];
    int n = tokenize(line, tokbuf, toks, SH_MAX_ARGS);
    if (n == -1) { eputs("sh: too many arguments\n"); return; }
    if (n == -2) { eputs("sh: unterminated quote\n"); return; }

    int argc = 0, append = 0;
    const char *redir = NULL;
    for (int i = 0; i < n; i++) {
        if (toks[i].op) {
            if (redir || i + 1 >= n || toks[i + 1].op) { eputs("sh: syntax error near '>'\n"); return; }
            append = toks[i].s[1] == '>';
            redir = toks[i + 1].s;
            i++;
        } else argv[argc++] = toks[i].s;
    }
    if (argc == 0) return;

    int found = 0;
    if (redir) { redirecting = 1; outlen = 0; }
    for (size_t i = 0; i < sizeof commands / sizeof commands[0]; i++) {
        if (ks_cmp(argv[0], commands[i].name) == 0) { commands[i].fn(argc, argv); found = 1; break; }
    }
    if (redir) {
        redirecting = 0;
        int node = fs_create(redir, sh_cwd);
        if (node < 0) err2("sh", redir, node);
        else { int e = fs_write(node, outbuf, outlen, append); if (e < 0) err2("sh", redir, e); }
    }
    if (!found) { eputs(argv[0]); eputs(": command not found\n"); }
    if (!fs_valid(sh_cwd)) sh_cwd = FS_ROOT;
}

/* ---- interactive loop ---- */
static void prompt(void) {
    char path[128];
    if (last_out != '\n') kputc('\n');
    kputs("user@something:");
    kputs(fs_path(sh_cwd, path, sizeof path) < 0 ? "?" : path);
    kputs("$ ");
}

static void readline(char *buf, int max) {
    int n = 0;
    for (;;) {
        int k = kbd_getkey();
        if (k == '\n') { kputc('\n'); buf[n] = 0; return; }
        if (k == 3) { kputs("^C\n"); buf[0] = 0; return; }
        if (k == 8) { if (n > 0) { n--; kputc('\b'); } continue; }
        if (k >= 32 && k < 127 && n < max - 1) { buf[n++] = (char)k; kputc((char)k); }
    }
}

void shell_run(void) {
    char line[SH_LINE_MAX];
    for (;;) {
        prompt();
        readline(line, sizeof line);
        last_out = '\n';
        shell_exec(line);
    }
}
