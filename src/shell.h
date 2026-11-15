#ifndef SHELL_H
#define SHELL_H
#define SH_HOME "/home/user"
extern int sh_cwd;                 /* current directory node */
void shell_exec(const char *line); /* run one command line */
void shell_run(void);              /* interactive loop (kernel only) */
#endif
