#ifndef GIT_OBJECTS_H
#define GIT_OBJECTS_H

#include "utils.h"

char* read_commit_object(const char *sha);
int write_commit_object(const char *sha, const char *data, size_t len);
int list_recent_commits(int n, commit_info_t *commits);
int get_commit_parent(const char *sha, char *parent_sha);
int is_tip_commit(const char *sha);
void update_branch_reference(const char *branch, const char *new_sha);

#endif