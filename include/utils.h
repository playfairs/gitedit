#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdint.h>

#define MAX_PATH 1024
#define MAX_BUFFER 65536
#define MAX_COMMITS 10
#define SHA1_HEX_SIZE 41
#define LOG_FILE "gitedit.log"

typedef struct {
    char sha[SHA1_HEX_SIZE];
    char author[256];
    char date[32];
    char message[256];
} commit_info_t;

int file_exists(const char *path);
size_t read_file(const char *filename, char *buffer, size_t max_len);
int write_file(const char *filename, const char *data, size_t len);
int copy_file(const char *src, const char *dst);

void trim_whitespace(char *str);
char* get_first_line(const char *buffer, char *line, size_t len);
void extract_commit_message(const char *buffer, char *message, size_t len);

void calculate_sha1(const char *data, size_t len, char *sha_hex);

void format_timestamp(time_t timestamp, char *buffer, size_t len);

void log_action(const char *action, const char *details);
void log_error(const char *error);

int is_git_repository(void);
int get_current_branch(char *branch, size_t len);

#endif