#include "../include/git_objects.h"
#include "../include/utils.h"
#include "../deps/miniz_simple.h"
#include <sys/stat.h>

static char* decompress_git_object(const char *object_path) {
    FILE *f = fopen(object_path, "rb");
    if (!f) return NULL;
    
    fseek(f, 0, SEEK_END);
    long file_size = ftell(f);
    fseek(f, 0, SEEK_SET);
    
    unsigned char *compressed_data = malloc(file_size);
    if (!compressed_data) {
        fclose(f);
        return NULL;
    }
    
    size_t compressed_len = fread(compressed_data, 1, file_size, f);
    fclose(f);
    
    size_t decompressed_size = MAX_BUFFER;
    char *decompressed_data = malloc(decompressed_size);
    if (!decompressed_data) {
        free(compressed_data);
        return NULL;
    }
    
    int result = mz_uncompress((unsigned char*)decompressed_data, &decompressed_size, 
                               compressed_data, compressed_len);
    
    free(compressed_data);
    
    if (result != 0) {
        free(decompressed_data);
        return NULL;
    }
    
    if (decompressed_size < MAX_BUFFER) {
        decompressed_data[decompressed_size] = '\0';
    } else {
        decompressed_data[MAX_BUFFER - 1] = '\0';
    }
    
    return decompressed_data;
}

static int compress_git_object(const char *data, size_t data_len, 
                              unsigned char **compressed_data, size_t *compressed_len) {
    *compressed_len = mz_compressBound(data_len);
    *compressed_data = malloc(*compressed_len);
    if (!*compressed_data) return 0;
    
    int result = mz_compress(*compressed_data, compressed_len, 
                            (const unsigned char*)data, data_len);
    
    if (result != 0) {
        free(*compressed_data);
        *compressed_data = NULL;
        return 0;
    }
    
    return 1;
}

char* read_commit_object(const char *sha) {
    char object_path[MAX_PATH];
    snprintf(object_path, sizeof(object_path), ".git/objects/%.2s/%s", sha, sha + 2);
    
    if (!file_exists(object_path)) {
        return NULL;
    }
    
    char *decompressed_data = decompress_git_object(object_path);
    if (!decompressed_data) {
        return NULL;
    }
    
    char *header_end = strchr(decompressed_data, '\0');
    if (!header_end) {
        free(decompressed_data);
        return NULL;
    }
    
    char *commit_data = header_end + 1;
    
    size_t commit_len = strlen(commit_data);
    char *result = malloc(commit_len + 1);
    if (result) {
        strcpy(result, commit_data);
    }
    
    free(decompressed_data);
    return result;
}

int write_commit_object(const char *sha, const char *data, size_t len) {
    char object_path[MAX_PATH];
    snprintf(object_path, sizeof(object_path), ".git/objects/%.2s/%s", sha, sha + 2);
    
    char dir_path[MAX_PATH];
    snprintf(dir_path, sizeof(dir_path), ".git/objects/%.2s", sha);
    mkdir(dir_path, 0755);
    
    char header[64];
    snprintf(header, sizeof(header), "commit %zu", len);
    size_t header_len = strlen(header) + 1;
    
    size_t total_size = header_len + len;
    char *full_data = malloc(total_size);
    if (!full_data) return 0;
    
    memcpy(full_data, header, header_len);
    memcpy(full_data + header_len, data, len);
    
    unsigned char *compressed_data;
    size_t compressed_len;
    
    if (!compress_git_object(full_data, total_size, &compressed_data, &compressed_len)) {
        free(full_data);
        return 0;
    }
    
    free(full_data);
    
    FILE *f = fopen(object_path, "wb");
    if (!f) {
        free(compressed_data);
        return 0;
    }
    
    size_t written = fwrite(compressed_data, 1, compressed_len, f);
    fclose(f);
    free(compressed_data);
    
    return (written == compressed_len);
}

int list_recent_commits(int n, commit_info_t *commits) {
    char branch[256];
    if (!get_current_branch(branch, sizeof(branch))) {
        return 0;
    }
    
    char head_path[MAX_PATH];
    snprintf(head_path, sizeof(head_path), ".git/refs/heads/%s", branch);
    
    char current_sha[SHA1_HEX_SIZE];
    FILE *f = fopen(head_path, "r");
    if (!f) return 0;
    
    if (fscanf(f, "%40s", current_sha) != 1) {
        fclose(f);
        return 0;
    }
    fclose(f);
    
    int count = 0;
    char sha[SHA1_HEX_SIZE];
    strcpy(sha, current_sha);
    
    while (count < n && strlen(sha) == 40) {
        char *commit_data = read_commit_object(sha);
        if (!commit_data) break;
        
        char *author_line = strstr(commit_data, "author ");
        char *date_str = NULL;
        
        if (author_line) {
            author_line += 7;
            char *author_end = strchr(author_line, '>');
            if (author_end) {
                *author_end = '\0';
                strncpy(commits[count].author, author_line, sizeof(commits[count].author) - 1);
                commits[count].author[sizeof(commits[count].author) - 1] = '\0';
                
                date_str = author_end + 1;
                while (*date_str && *date_str == ' ') date_str++;
                
                if (*date_str) {
                    time_t timestamp = atol(date_str);
                    format_timestamp(timestamp, commits[count].date, sizeof(commits[count].date));
                }
            }
        }
        
        extract_commit_message(commit_data, commits[count].message, sizeof(commits[count].message));
        strcpy(commits[count].sha, sha);
        
        free(commit_data);
        count++;
        
        if (!get_commit_parent(sha, sha)) break;
    }
    
    return count;
}

int get_commit_parent(const char *sha, char *parent_sha) {
    char *commit_data = read_commit_object(sha);
    if (!commit_data) return 0;
    
    char *parent_line = strstr(commit_data, "parent ");
    if (!parent_line) {
        free(commit_data);
        return 0;
    }
    
    parent_line += 7;
    if (sscanf(parent_line, "%40s", parent_sha) != 1) {
        free(commit_data);
        return 0;
    }
    
    free(commit_data);
    return 1;
}

int is_tip_commit(const char *sha) {
    char branch[256];
    if (!get_current_branch(branch, sizeof(branch))) {
        return 0;
    }
    
    char head_path[MAX_PATH];
    snprintf(head_path, sizeof(head_path), ".git/refs/heads/%s", branch);
    
    char current_sha[SHA1_HEX_SIZE];
    FILE *f = fopen(head_path, "r");
    if (!f) return 0;
    
    if (fscanf(f, "%40s", current_sha) != 1) {
        fclose(f);
        return 0;
    }
    fclose(f);
    
    return (strcmp(sha, current_sha) == 0);
}

void update_branch_reference(const char *branch, const char *new_sha) {
    char head_path[MAX_PATH];
    snprintf(head_path, sizeof(head_path), ".git/refs/heads/%s", branch);
    
    FILE *f = fopen(head_path, "w");
    if (f) {
        fprintf(f, "%s\n", new_sha);
        fclose(f);
        
        char log_details[512];
        snprintf(log_details, sizeof(log_details), "Updated branch %s to %s", branch, new_sha);
        log_action("BRANCH_UPDATED", log_details);
    }
}
