#include "../include/utils.h"
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>

int file_exists(const char *path) {
    struct stat st;
    return (stat(path, &st) == 0);
}

size_t read_file(const char *filename, char *buffer, size_t max_len) {
    FILE *f = fopen(filename, "rb");
    if (!f) return 0;
    
    size_t len = fread(buffer, 1, max_len - 1, f);
    fclose(f);
    buffer[len] = '\0';
    return len;
}

int write_file(const char *filename, const char *data, size_t len) {
    FILE *f = fopen(filename, "wb");
    if (!f) return 0;
    
    size_t written = fwrite(data, 1, len, f);
    fclose(f);
    return (written == len);
}

int copy_file(const char *src, const char *dst) {
    char buffer[4096];
    FILE *fsrc = fopen(src, "rb");
    FILE *fdst = fopen(dst, "wb");
    
    if (!fsrc || !fdst) {
        if (fsrc) fclose(fsrc);
        if (fdst) fclose(fdst);
        return 0;
    }
    
    size_t bytes;
    while ((bytes = fread(buffer, 1, sizeof(buffer), fsrc)) > 0) {
        if (fwrite(buffer, 1, bytes, fdst) != bytes) {
            fclose(fsrc);
            fclose(fdst);
            return 0;
        }
    }
    
    fclose(fsrc);
    fclose(fdst);
    return 1;
}

void trim_whitespace(char *str) {
    char *end;
    
    while (*str == ' ' || *str == '\t' || *str == '\n' || *str == '\r') str++;
    
    if (*str == 0) return;
    
    end = str + strlen(str) - 1;
    while (end > str && (*end == ' ' || *end == '\t' || *end == '\n' || *end == '\r')) end--;
    
    *(end + 1) = '\0';
}

char* get_first_line(const char *buffer, char *line, size_t len) {
    const char *p = strchr(buffer, '\n');
    if (p) {
        size_t line_len = p - buffer;
        if (line_len >= len) line_len = len - 1;
        strncpy(line, buffer, line_len);
        line[line_len] = '\0';
    } else {
        strncpy(line, buffer, len - 1);
        line[len - 1] = '\0';
    }
    trim_whitespace(line);
    return line;
}

void extract_commit_message(const char *buffer, char *message, size_t len) {
    const char *p = strstr(buffer, "\n\n");
    if (p) {
        const char *msg_start = p + 2;
        const char *msg_end = strchr(msg_start, '\n');
        if (msg_end) {
            size_t msg_len = msg_end - msg_start;
            if (msg_len >= len) msg_len = len - 1;
            strncpy(message, msg_start, msg_len);
            message[msg_len] = '\0';
        } else {
            strncpy(message, msg_start, len - 1);
            message[len - 1] = '\0';
        }
    } else {
        strcpy(message, "(no message)");
    }
    trim_whitespace(message);
}

void calculate_sha1(const char *data, size_t len, char *sha_hex) {
    unsigned int hash = 0;
    for (size_t i = 0; i < len && i < 64; i++) {
        hash = hash * 31 + (unsigned char)data[i];
    }
    
    snprintf(sha_hex, SHA1_HEX_SIZE, "%08x%08x%08x%08x%08x", 
             hash, hash ^ 0x12345678, hash ^ 0x87654321, 
             hash ^ 0xABCDEF00, hash ^ 0x00FEDCBA);
    sha_hex[40] = '\0';
}

void format_timestamp(time_t timestamp, char *buffer, size_t len) {
    struct tm *tm_info = localtime(&timestamp);
    strftime(buffer, len, "%Y-%m-%d %H:%M:%S", tm_info);
}

void log_action(const char *action, const char *details) {
    FILE *log = fopen(LOG_FILE, "a");
    if (log) {
        time_t now = time(NULL);
        char timestamp[32];
        format_timestamp(now, timestamp, sizeof(timestamp));
        fprintf(log, "[%s] ACTION: %s - %s\n", timestamp, action, details);
        fclose(log);
    }
}

void log_error(const char *error) {
    FILE *log = fopen(LOG_FILE, "a");
    if (log) {
        time_t now = time(NULL);
        char timestamp[32];
        format_timestamp(now, timestamp, sizeof(timestamp));
        fprintf(log, "[%s] ERROR: %s\n", timestamp, error);
        fclose(log);
    }
}

int is_git_repository(void) {
    return file_exists(".git");
}

int get_current_branch(char *branch, size_t len) {
    char head_content[256];
    size_t head_len = read_file(".git/HEAD", head_content, sizeof(head_content));
    
    if (head_len == 0) return 0;
    
    const char *ref_prefix = "ref: refs/heads/";
    if (strncmp(head_content, ref_prefix, strlen(ref_prefix)) == 0) {
        const char *branch_name = head_content + strlen(ref_prefix);
        trim_whitespace((char*)branch_name);
        strncpy(branch, branch_name, len - 1);
        branch[len - 1] = '\0';
        return 1;
    }
    
    return 0;
}
