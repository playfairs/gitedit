#include "../include/backup.h"
#include "../include/utils.h"
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <unistd.h>

static int copy_directory_recursive(const char *src, const char *dst) {
    DIR *dir;
    struct dirent *entry;
    struct stat statbuf;
    char src_path[MAX_PATH];
    char dst_path[MAX_PATH];
    
    if (mkdir(dst, 0755) != 0 && errno != EEXIST) {
        return 0;
    }
    
    dir = opendir(src);
    if (!dir) return 0;
    
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        
        snprintf(src_path, sizeof(src_path), "%s/%s", src, entry->d_name);
        snprintf(dst_path, sizeof(dst_path), "%s/%s", dst, entry->d_name);
        
        if (stat(src_path, &statbuf) == -1) {
            closedir(dir);
            return 0;
        }
        
        if (S_ISDIR(statbuf.st_mode)) {
            if (!copy_directory_recursive(src_path, dst_path)) {
                closedir(dir);
                return 0;
            }
        } else {
            if (!copy_file(src_path, dst_path)) {
                closedir(dir);
                return 0;
            }
        }
    }
    
    closedir(dir);
    return 1;
}

static int remove_directory_recursive(const char *path) {
    DIR *dir;
    struct dirent *entry;
    struct stat statbuf;
    char full_path[MAX_PATH];
    
    dir = opendir(path);
    if (!dir) return 0;
    
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        
        snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);
        
        if (stat(full_path, &statbuf) == -1) {
            closedir(dir);
            return 0;
        }
        
        if (S_ISDIR(statbuf.st_mode)) {
            if (!remove_directory_recursive(full_path)) {
                closedir(dir);
                return 0;
            }
        } else {
            if (unlink(full_path) != 0) {
                closedir(dir);
                return 0;
            }
        }
    }
    
    closedir(dir);
    return (rmdir(path) == 0);
}

int backup_git(void) {
    char backup_path[MAX_PATH];
    char timestamp[32];
    
    if (!is_git_repository()) {
        log_error("Not in a git repository");
        return 0;
    }
    
    get_backup_timestamp(timestamp, sizeof(timestamp));
    snprintf(backup_path, sizeof(backup_path), ".git_backup_%s", timestamp);
    
    printf("Creating backup: %s\n", backup_path);
    
    if (copy_directory_recursive(".git", backup_path)) {
        char log_details[256];
        snprintf(log_details, sizeof(log_details), "Created backup at %s", backup_path);
        log_action("BACKUP_CREATED", log_details);
        return 1;
    } else {
        log_error("Failed to create backup");
        return 0;
    }
}

int restore_git_backup(const char *backup_name) {
    char backup_path[MAX_PATH];
    
    snprintf(backup_path, sizeof(backup_path), ".git_backup_%s", backup_name);
    
    if (!file_exists(backup_path)) {
        printf("Backup not found: %s\n", backup_path);
        return 0;
    }
    
    printf("WARNING: This will replace your current .git directory with backup %s\n", backup_name);
    printf("This action cannot be undone. Continue? (y/N): ");
    
    char response[10];
    if (fgets(response, sizeof(response), stdin) == NULL || 
        (response[0] != 'y' && response[0] != 'Y')) {
        printf("Restore cancelled.\n");
        return 0;
    }
    
    if (file_exists(".git")) {
        if (!remove_directory_recursive(".git")) {
            log_error("Failed to remove current .git directory");
            return 0;
        }
    }
    
    if (copy_directory_recursive(backup_path, ".git")) {
        char log_details[256];
        snprintf(log_details, sizeof(log_details), "Restored from backup %s", backup_name);
        log_action("BACKUP_RESTORED", log_details);
        printf("Backup restored successfully.\n");
        return 1;
    } else {
        log_error("Failed to restore backup");
        return 0;
    }
}

int list_backups(void) {
    DIR *dir;
    struct dirent *entry;
    int found = 0;
    
    dir = opendir(".");
    if (!dir) return 0;
    
    printf("Available backups:\n");
    
    while ((entry = readdir(dir)) != NULL) {
        if (strncmp(entry->d_name, ".git_backup_", 12) == 0) {
            printf("  %s\n", entry->d_name + 12);
            found = 1;
        }
    }
    
    closedir(dir);
    
    if (!found) {
        printf("No backups found.\n");
    }
    
    return found;
}

void get_backup_timestamp(char *timestamp, size_t len) {
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    strftime(timestamp, len, "%Y%m%d_%H%M%S", tm_info);
}
