#ifndef BACKUP_H
#define BACKUP_H

#include <time.h>

int backup_git(void);
int restore_git_backup(const char *backup_name);
int list_backups(void);
void get_backup_timestamp(char *timestamp, size_t len);

#endif