#ifndef UI_H
#define UI_H
/* Prompt on the status row. Returns 1 if the user entered a non-empty line, 0 if cancelled (Esc). */
int ui_prompt(const char *label, char *out, int max);
void ui_status(const char *msg);     /* message on row 23 */
#define UI_STATUS_ROW 23
#define UI_HELP_ROW 24
#endif
