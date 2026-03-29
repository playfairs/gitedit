#ifndef TUI_H
#define TUI_H

#include "utils.h"
#include <ncurses.h>

typedef struct {
    int selected;
    int count;
    commit_info_t *commits;
} tui_state_t;

int tui_init(void);
void tui_cleanup(void);
int tui_run_interface(commit_info_t *commits, int count, int *selection);
char* tui_edit_message(const char *current_message);
int tui_confirm_dialog(const char *message);
void tui_show_error(const char *message);
void tui_show_success(const char *message);

#endif
