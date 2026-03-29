#ifndef EDITOR_H
#define EDITOR_H

#include "utils.h"

int display_commits_and_select(commit_info_t *commits, int count, int *selection);
char* edit_commit_message(const char *current_message);
int confirm_action(const char *message);

#endif