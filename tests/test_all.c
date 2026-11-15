#include <stdio.h>
#include <string.h>
#include "fs.h"
#include "shell.h"
#include "keymap.h"
#include "editor_core.h"

extern char out_log[];
extern int editor_calls, fm_calls, clear_calls, reboot_calls, halt_calls;
extern char editor_arg[];
void out_reset(void);

static int failures, checks;
#define CHECK(cond) do { checks++; if (!(cond)) { failures++; printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond); } } while (0)
#define CHECK_EQ(a, b) do { checks++; long _a = (long)(a), _b = (long)(b); if (_a != _b) { failures++; printf("FAIL %s:%d: %s == %s (%ld vs %ld)\n", __FILE__, __LINE__, #a, #b, _a, _b); } } while (0)
#define CHECK_STR(a, b) do { checks++; if (strcmp((a), (b)) != 0) { failures++; printf("FAIL %s:%d: got \"%s\" expected \"%s\"\n", __FILE__, __LINE__, (a), (b)); } } while (0)

static void reset_world(void) { fs_init(); sh_cwd = FS_ROOT; out_reset(); }
static const char *run(const char *cmd) { out_reset(); shell_exec(cmd); return out_log; }

/* ---------- file system ---------- */
static void test_fs_paths(void) {
    reset_world();
    CHECK_EQ(fs_mkdir("/a", FS_ROOT), 0);
    CHECK_EQ(fs_mkdir("a/b", FS_ROOT), 0);
    int a = fs_resolve("/a", FS_ROOT), b = fs_resolve("/a/b", FS_ROOT);
    CHECK(a > 0 && b > 0 && a != b);
    CHECK_EQ(fs_resolve("b", a), b);
    CHECK_EQ(fs_resolve(".", b), b);
    CHECK_EQ(fs_resolve("..", b), a);
    CHECK_EQ(fs_resolve("../..", b), FS_ROOT);
    CHECK_EQ(fs_resolve("../../..", b), FS_ROOT);          /* root's parent is root */
    CHECK_EQ(fs_resolve("/a//b/", FS_ROOT), b);            /* extra and trailing slashes */
    CHECK_EQ(fs_resolve("/", b), FS_ROOT);
    CHECK_EQ(fs_resolve("/nope", FS_ROOT), FS_ENOENT);
    CHECK_EQ(fs_mkdir("/a", FS_ROOT), FS_EEXIST);
    CHECK_EQ(fs_mkdir("/x/y", FS_ROOT), FS_ENOENT);
    CHECK_EQ(fs_mkdir("/", FS_ROOT), FS_EINVAL);
    CHECK_EQ(fs_mkdir("/a/.", FS_ROOT), FS_EINVAL);
    CHECK_EQ(fs_mkdir("/a/..", FS_ROOT), FS_EINVAL);
    CHECK_EQ(fs_mkdir("", FS_ROOT), FS_EINVAL);
    char longname[64]; memset(longname, 'x', 40); longname[40] = 0;
    CHECK_EQ(fs_mkdir(longname, FS_ROOT), FS_ENAMETOOLONG);
    char buf[64];
    CHECK_EQ(fs_path(b, buf, sizeof buf), 0); CHECK_STR(buf, "/a/b");
    CHECK_EQ(fs_path(FS_ROOT, buf, sizeof buf), 0); CHECK_STR(buf, "/");
    CHECK_EQ(fs_path(b, buf, 3), FS_ENAMETOOLONG);
    CHECK_EQ(fs_mkdir_p("/p/q/r", FS_ROOT), 0);
    CHECK(fs_resolve("/p/q/r", FS_ROOT) > 0);
    CHECK_EQ(fs_mkdir_p("/p/q/r", FS_ROOT), 0);            /* idempotent */
}

static void test_fs_files(void) {
    reset_world();
    int f = fs_create("/f.txt", FS_ROOT);
    CHECK(f > 0);
    CHECK_EQ(fs_create("/f.txt", FS_ROOT), f);             /* existing file: same node */
    CHECK_EQ(fs_write(f, "hello", 5, 0), 0);
    CHECK_EQ(fs_size(f), 5);
    CHECK(memcmp(fs_data(f), "hello", 5) == 0);
    CHECK_EQ(fs_write(f, " world", 6, 1), 0);
    CHECK_EQ(fs_size(f), 11);
    CHECK(memcmp(fs_data(f), "hello world", 11) == 0);
    CHECK_EQ(fs_write(f, "x", 1, 0), 0);                   /* overwrite shrinks */
    CHECK_EQ(fs_size(f), 1);
    char big[FS_FILE_MAX + 1]; memset(big, 'a', sizeof big);
    CHECK_EQ(fs_write(f, big, FS_FILE_MAX, 0), 0);
    CHECK_EQ(fs_write(f, big, FS_FILE_MAX + 1, 0), FS_ENOSPC);
    CHECK_EQ(fs_write(f, "z", 1, 1), FS_ENOSPC);           /* append past the limit */
    CHECK_EQ(fs_write(f, "", 0, 0), 0);
    CHECK_EQ(fs_size(f), 0);
    fs_mkdir("/d", FS_ROOT);
    CHECK_EQ(fs_create("/d", FS_ROOT), FS_EISDIR);
    CHECK_EQ(fs_write(fs_resolve("/d", FS_ROOT), "x", 1, 0), FS_EISDIR);
    CHECK_EQ(fs_create("/f.txt/inner", FS_ROOT), FS_ENOTDIR);
    CHECK_EQ(fs_resolve("/f.txt/inner", FS_ROOT), FS_ENOTDIR);
}

static void test_fs_remove_and_limits(void) {
    reset_world();
    int free0 = fs_free_blocks();
    fs_mkdir_p("/t/u", FS_ROOT);
    int f = fs_create("/t/u/file", FS_ROOT);
    fs_write(f, "data", 4, 0);
    CHECK_EQ(fs_free_blocks(), free0 - 1);
    CHECK_EQ(fs_remove_node(fs_resolve("/t", FS_ROOT), 0), FS_ENOTEMPTY);
    CHECK_EQ(fs_remove_node(fs_resolve("/t", FS_ROOT), 1), 0);
    CHECK_EQ(fs_free_blocks(), free0);                     /* blocks released */
    CHECK_EQ(fs_resolve("/t", FS_ROOT), FS_ENOENT);
    CHECK_EQ(fs_remove_node(FS_ROOT, 1), FS_EINVAL);

    reset_world();                                         /* block exhaustion */
    int created = 0, err = 0;
    for (int i = 0; i < FS_BLOCKS + 5; i++) {
        char name[16]; snprintf(name, sizeof name, "f%d", i);
        int n = fs_create(name, FS_ROOT);
        if (n < 0) { err = n; break; }
        int e = fs_write(n, "x", 1, 0);
        if (e < 0) { err = e; break; }
        created++;
    }
    CHECK_EQ(created, FS_BLOCKS);
    CHECK_EQ(err, FS_ENOSPC);
    reset_world();                                         /* node exhaustion */
    int made = 0;
    for (int i = 0; i < FS_MAX_NODES + 5; i++) {
        char name[16]; snprintf(name, sizeof name, "n%d", i);
        if (fs_create(name, FS_ROOT) < 0) break;
        made++;
    }
    CHECK_EQ(made, FS_MAX_NODES - 1);                      /* root takes one slot */
}

static void test_fs_children_copy_move(void) {
    reset_world();
    fs_create("/c", FS_ROOT); fs_create("/a", FS_ROOT); fs_mkdir("/b", FS_ROOT);
    int kids[8];
    int n = fs_children(FS_ROOT, kids, 8);
    CHECK_EQ(n, 3);
    CHECK_STR(fs_name(kids[0]), "a"); CHECK_STR(fs_name(kids[1]), "b"); CHECK_STR(fs_name(kids[2]), "c");

    int a = fs_resolve("/a", FS_ROOT);
    fs_write(a, "abc", 3, 0);
    CHECK_EQ(fs_copy("/a", "/a2", FS_ROOT), 0);
    CHECK(memcmp(fs_data(fs_resolve("/a2", FS_ROOT)), "abc", 3) == 0);
    CHECK_EQ(fs_copy("/a", "/b", FS_ROOT), 0);             /* into a directory */
    CHECK(fs_resolve("/b/a", FS_ROOT) > 0);
    CHECK_EQ(fs_copy("/a", "/a", FS_ROOT), FS_EINVAL);
    CHECK_EQ(fs_copy("/b", "/x", FS_ROOT), FS_EISDIR);
    CHECK_EQ(fs_copy("/missing", "/x", FS_ROOT), FS_ENOENT);

    CHECK_EQ(fs_move("/a2", "/b", FS_ROOT), 0);
    CHECK_EQ(fs_resolve("/a2", FS_ROOT), FS_ENOENT);
    CHECK(fs_resolve("/b/a2", FS_ROOT) > 0);
    CHECK_EQ(fs_move("/b/a2", "/renamed", FS_ROOT), 0);
    CHECK(fs_resolve("/renamed", FS_ROOT) > 0);
    CHECK_EQ(fs_move("/c", "/renamed", FS_ROOT), FS_EEXIST);
    fs_mkdir("/b/deep", FS_ROOT);
    CHECK_EQ(fs_move("/b", "/b/deep", FS_ROOT), FS_EINVAL); /* dir into itself/descendant */
    CHECK_EQ(fs_move("/b", "/b", FS_ROOT), FS_EINVAL);
    CHECK_EQ(fs_move("/", "/b", FS_ROOT), FS_EINVAL);

    int r = fs_resolve("/renamed", FS_ROOT);
    CHECK_EQ(fs_rename_node(r, "new"), 0);
    CHECK_STR(fs_name(r), "new");
    CHECK_EQ(fs_rename_node(r, "c"), FS_EEXIST);
    CHECK_EQ(fs_rename_node(r, "a/b"), FS_EINVAL);
    CHECK_EQ(fs_rename_node(r, ".."), FS_EINVAL);
}

/* ---------- shell ---------- */
static void test_shell_navigation(void) {
    reset_world();
    fs_mkdir_p("/home/user", FS_ROOT);
    CHECK_STR(run("pwd"), "/\n");
    CHECK_STR(run("mkdir proj"), "");
    CHECK_STR(run("ls"), "home/  proj/  \n");
    run("cd proj");
    CHECK_STR(run("pwd"), "/proj\n");
    run("mkdir -p a/b/c");
    run("cd a/b/c");
    CHECK_STR(run("pwd"), "/proj/a/b/c\n");
    run("cd ../..");
    CHECK_STR(run("pwd"), "/proj/a\n");
    run("cd /");
    CHECK_STR(run("pwd"), "/\n");
    run("cd");                                             /* no argument: home */
    CHECK_STR(run("pwd"), "/home/user\n");
    CHECK_STR(run("cd nonexistent"), "cd: nonexistent: No such file or directory\n");
    run("touch file");
    CHECK_STR(run("cd file"), "cd: file: Not a directory\n");
    CHECK_STR(run("mkdir /home"), "mkdir: /home: File exists\n");
    CHECK_STR(run("mkdir"), "mkdir: missing operand\n");
}

static void test_shell_files(void) {
    reset_world();
    run("echo hi > f");
    CHECK_STR(run("cat f"), "hi\n");
    run("echo more >> f");
    CHECK_STR(run("cat f"), "hi\nmore\n");
    run("echo new > f");
    CHECK_STR(run("cat f"), "new\n");                      /* > truncates */
    CHECK_STR(run("echo \"a   b\""), "a   b\n");           /* quotes keep spaces */
    CHECK_STR(run("echo a   b"), "a b\n");
    CHECK_STR(run("echo -n x"), "x");
    CHECK_STR(run("echo hi>g"), "");                       /* no spaces around > */
    CHECK_STR(run("cat g"), "hi\n");
    CHECK_STR(run("echo \">\" > h"), "");                  /* quoted > is data */
    CHECK_STR(run("cat h"), ">\n");
    CHECK_STR(run("ls -l"), "-      4  f\n-      3  g\n-      2  h\n");
    run("echo data > out.txt");
    CHECK_STR(run("ls > listing"), "");                    /* redirect works for any command */
    CHECK(strstr(run("cat listing"), "out.txt") != NULL);
}

static void test_shell_manage(void) {
    reset_world();
    run("mkdir d");
    run("echo x > d/f");
    CHECK_STR(run("rm d"), "rm: d: Is a directory\n");
    CHECK_STR(run("rmdir d"), "rmdir: d: Directory not empty\n");
    CHECK_STR(run("rmdir d/f"), "rmdir: d/f: Not a directory\n");
    CHECK_STR(run("rm d/f"), "");
    CHECK_STR(run("rmdir d"), "");
    CHECK_STR(run("cat d"), "cat: d: No such file or directory\n");
    CHECK_STR(run("rm nothing"), "rm: nothing: No such file or directory\n");
    CHECK_STR(run("rm -f nothing"), "");
    run("mkdir -p t/u"); run("touch t/u/x");
    CHECK_STR(run("rm -r t"), "");
    CHECK_STR(run("ls"), "");
    CHECK_STR(run("rm -r /"), "rm: /: Invalid argument\n");

    run("echo abc > a");
    CHECK_STR(run("cp a b"), "");
    CHECK_STR(run("cat b"), "abc\n");
    run("mkdir dir");
    run("mv b dir");
    CHECK_STR(run("cat dir/b"), "abc\n");
    run("mv a z");
    CHECK_STR(run("cat z"), "abc\n");
    CHECK_STR(run("cp onlyone"), "cp: usage: cp SOURCE DEST\n");
    CHECK_STR(run("cat"), "cat: missing file operand\n");
    CHECK_STR(run("touch"), "touch: missing operand\n");

    /* deleting the current directory drops the shell back to root */
    run("mkdir gone"); run("cd gone");
    CHECK(sh_cwd != FS_ROOT);
    run("rm -r /gone");
    CHECK_EQ(sh_cwd, FS_ROOT);
}

static void test_shell_misc(void) {
    reset_world();
    CHECK_STR(run("frobnicate"), "frobnicate: command not found\n");
    CHECK_STR(run(""), "");
    CHECK_STR(run("   "), "");
    CHECK_STR(run("echo \"unterminated"), "sh: unterminated quote\n");
    CHECK_STR(run("echo >"), "sh: syntax error near '>'\n");
    CHECK_STR(run("echo a > > b"), "sh: syntax error near '>'\n");
    CHECK_STR(run("echo a > f > g"), "sh: syntax error near '>'\n");
    CHECK_STR(run("a b c d e f g h i j k l m n o p q"), "sh: too many arguments\n");
    CHECK_STR(run("uname"), "Something Project 0.1.0 x86_64\n");
    CHECK(strstr(run("help"), "nano") != NULL);
    CHECK_STR(run("df"), "blocks (4096 bytes): used 0 / total 192\n");
    run("echo x > f");
    CHECK_STR(run("df"), "blocks (4096 bytes): used 1 / total 192\n");

    int e0 = editor_calls, f0 = fm_calls, c0 = clear_calls, r0 = reboot_calls, h0 = halt_calls;
    run("nano notes.txt");
    CHECK_EQ(editor_calls, e0 + 1);
    CHECK_STR(editor_arg, "notes.txt");
    run("edit");
    CHECK_STR(editor_arg, "");
    run("fm");
    CHECK_EQ(fm_calls, f0 + 1);
    CHECK(clear_calls > c0);
    run("reboot"); run("halt");
    CHECK_EQ(reboot_calls, r0 + 1);
    CHECK_EQ(halt_calls, h0 + 1);

    run("echo x > big");                                   /* output larger than a file is truncated */
    for (int i = 0; i < 300; i++) run("echo aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa >> big");
    CHECK(fs_size(fs_resolve("big", FS_ROOT)) <= FS_FILE_MAX);
}

/* ---------- keyboard ---------- */
static int press(kbd_state *s, unsigned char sc) { int k = kbd_feed(s, sc); kbd_feed(s, sc | 0x80); return k; }

static void test_keymap(void) {
    kbd_state s; kbd_state_init(&s);
    CHECK_EQ(press(&s, 0x1E), 'a');
    CHECK_EQ(press(&s, 0x10), 'q');
    CHECK_EQ(press(&s, 0x39), ' ');
    CHECK_EQ(press(&s, 0x1C), '\n');
    CHECK_EQ(press(&s, 0x0E), '\b');
    CHECK_EQ(press(&s, 0x35), '/');
    CHECK_EQ(press(&s, 0x02), '1');
    CHECK_EQ(press(&s, 0x01), 27);

    kbd_feed(&s, 0x2A);                                    /* shift down */
    CHECK_EQ(kbd_feed(&s, 0x1E), 'A');
    CHECK_EQ(kbd_feed(&s, 0x02), '!');
    CHECK_EQ(kbd_feed(&s, 0x35), '?');
    kbd_feed(&s, 0xAA);                                    /* shift up */
    CHECK_EQ(press(&s, 0x1E), 'a');

    press(&s, 0x3A);                                       /* caps lock on */
    CHECK_EQ(press(&s, 0x1E), 'A');
    CHECK_EQ(press(&s, 0x02), '1');                        /* caps does not affect digits */
    kbd_feed(&s, 0x36);
    CHECK_EQ(kbd_feed(&s, 0x1E), 'a');                     /* caps + shift = lowercase */
    kbd_feed(&s, 0xB6);
    press(&s, 0x3A);                                       /* caps lock off */
    CHECK_EQ(press(&s, 0x1E), 'a');

    kbd_feed(&s, 0x1D);                                    /* ctrl down */
    CHECK_EQ(kbd_feed(&s, 0x18), 15);                      /* ^O */
    CHECK_EQ(kbd_feed(&s, 0x2D), 24);                      /* ^X */
    CHECK_EQ(kbd_feed(&s, 0x25), 11);                      /* ^K */
    CHECK_EQ(kbd_feed(&s, 0x16), 21);                      /* ^U */
    kbd_feed(&s, 0x9D);
    CHECK_EQ(press(&s, 0x18), 'o');

    CHECK_EQ(kbd_feed(&s, 0xE0) | kbd_feed(&s, 0x48), KEY_UP);
    kbd_feed(&s, 0xE0); kbd_feed(&s, 0xC8);                /* release of up */
    kbd_feed(&s, 0xE0); CHECK_EQ(kbd_feed(&s, 0x50), KEY_DOWN);
    kbd_feed(&s, 0xE0); CHECK_EQ(kbd_feed(&s, 0x4B), KEY_LEFT);
    kbd_feed(&s, 0xE0); CHECK_EQ(kbd_feed(&s, 0x4D), KEY_RIGHT);
    kbd_feed(&s, 0xE0); CHECK_EQ(kbd_feed(&s, 0x53), KEY_DEL);
    kbd_feed(&s, 0xE0); CHECK_EQ(kbd_feed(&s, 0x47), KEY_HOME);
    kbd_feed(&s, 0xE0); CHECK_EQ(kbd_feed(&s, 0x4F), KEY_END);
    kbd_feed(&s, 0xE0); CHECK_EQ(kbd_feed(&s, 0x49), KEY_PGUP);
    kbd_feed(&s, 0xE0); CHECK_EQ(kbd_feed(&s, 0x51), KEY_PGDN);
    CHECK_EQ(press(&s, 0x5A), 0);                          /* unknown scancode ignored */
    CHECK_EQ(press(&s, 0x1E), 'a');                        /* decoder still in sync afterwards */

    /* every letter key maps to its own letter, and shift gives the capital */
    const char *rows[] = { "qwertyuiop", "asdfghjkl", "zxcvbnm" };
    unsigned char start[] = { 0x10, 0x1E, 0x2C };
    for (int r = 0; r < 3; r++)
        for (int i = 0; rows[r][i]; i++) {
            kbd_state t; kbd_state_init(&t);
            CHECK_EQ(press(&t, (unsigned char)(start[r] + i)), rows[r][i]);
            kbd_feed(&t, 0x2A);
            CHECK_EQ(kbd_feed(&t, (unsigned char)(start[r] + i)), rows[r][i] - 32);
        }
}

/* ---------- editor buffer ---------- */
static void test_editor_core(void) {
    static ebuf e;
    eb_init(&e, "", 0);
    for (const char *p = "hello\nworld"; *p; p++) eb_insert(&e, *p);
    CHECK_EQ(e.len, 11); CHECK_EQ(e.cur, 11); CHECK(e.modified);
    CHECK_EQ(eb_line(&e), 1); CHECK_EQ(eb_col(&e), 5);
    eb_up(&e);
    CHECK_EQ(e.cur, 5); CHECK_EQ(eb_line(&e), 0);          /* column preserved (end of "hello") */
    eb_down(&e);
    CHECK_EQ(e.cur, 11);
    eb_home(&e); CHECK_EQ(e.cur, 6);
    eb_end(&e); CHECK_EQ(e.cur, 11);
    eb_up(&e); eb_up(&e);                                  /* already on first line: stays */
    CHECK_EQ(eb_line(&e), 0);
    eb_home(&e);
    eb_backspace(&e);                                      /* at start: nothing */
    CHECK_EQ(e.len, 11);
    eb_right(&e); eb_right(&e);
    eb_backspace(&e);
    CHECK(memcmp(e.data, "hllo\nworld", 10) == 0); CHECK_EQ(e.len, 10);
    eb_delete(&e);
    CHECK(memcmp(e.data, "hlo\nworld", 9) == 0);
    eb_end(&e); eb_delete(&e);                             /* delete the newline: lines join */
    CHECK(memcmp(e.data, "hloworld", 8) == 0); CHECK_EQ(e.len, 8);

    eb_init(&e, "short\na much longer line\nend", 28);     /* column clamped on shorter lines */
    eb_down(&e); eb_end(&e);
    CHECK_EQ(eb_col(&e), 18);
    eb_down(&e);
    CHECK_EQ(eb_line(&e), 2); CHECK_EQ(eb_col(&e), 3);
    eb_up(&e); eb_up(&e);
    CHECK_EQ(eb_line(&e), 0); CHECK_EQ(eb_col(&e), 3);

    char clip[64];
    eb_init(&e, "one\ntwo\nthree", 13);
    eb_down(&e);
    int n = eb_cut_line(&e, clip, sizeof clip);
    CHECK_EQ(n, 4); CHECK(memcmp(clip, "two\n", 4) == 0);
    CHECK(memcmp(e.data, "one\nthree", 9) == 0); CHECK_EQ(e.len, 9);
    CHECK_EQ(eb_line(&e), 1);
    eb_paste(&e, clip, n);
    CHECK(memcmp(e.data, "one\ntwo\nthree", 13) == 0);
    eb_init(&e, "", 0);
    CHECK_EQ(eb_cut_line(&e, clip, sizeof clip), 0);       /* empty buffer: nothing */

    eb_init(&e, "", 0);                                    /* capacity */
    int ok = 0;
    for (int i = 0; i < FS_FILE_MAX + 10; i++) if (eb_insert(&e, 'x') == 0) ok++;
    CHECK_EQ(ok, FS_FILE_MAX);
    CHECK_EQ(eb_insert(&e, 'y'), -1);
    eb_init(&e, "abc", 3);
    CHECK(!e.modified);
}

int main(void) {
    test_fs_paths();
    test_fs_files();
    test_fs_remove_and_limits();
    test_fs_children_copy_move();
    test_shell_navigation();
    test_shell_files();
    test_shell_manage();
    test_shell_misc();
    test_keymap();
    test_editor_core();
    printf("%d checks, %d failures\n", checks, failures);
    return failures ? 1 : 0;
}
