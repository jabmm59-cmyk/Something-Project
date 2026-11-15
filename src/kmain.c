#include "console.h"
#include "fs.h"
#include "io.h"
#include "plat.h"
#include "shell.h"

void plat_reboot(void) {
    for (int i = 0; i < 100000 && (inb(0x64) & 2); i++) { }
    outb(0x64, 0xFE);                         /* pulse the CPU reset line */
    for (;;) __asm__ volatile("cli; hlt");
}

void plat_halt(void) {
    kputs("System halted. You can power off now.\n");
    for (;;) __asm__ volatile("cli; hlt");
}

static const char readme[] =
    "Welcome to Something Project!\n"
    "\n"
    "Try:  ls, cd, mkdir, nano notes.txt, fm, help\n"
    "The file system lives in RAM: files are lost on reboot.\n";

void kmain(void) {
    con_clear();
    fs_init();
    fs_mkdir_p("/home/user", FS_ROOT);
    fs_mkdir_p("/tmp", FS_ROOT);
    int n = fs_create("/home/user/readme.txt", FS_ROOT);
    if (n >= 0) fs_write(n, readme, sizeof readme - 1, 0);
    int home = fs_resolve(SH_HOME, FS_ROOT);
    if (home >= 0) sh_cwd = home;

    kputs("Something Project 0.1.0 (x86_64)\n");
    kputs("Type 'help' for a list of commands.\n\n");
    shell_run();
}
