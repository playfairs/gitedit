#include "../include/utils.h"
#include "../include/backup.h"
#include "../include/git_objects.h"
#include "../include/editor.h"
#include "../include/tui.h"
#include "../include/process.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

static volatile int interrupted = 0;

void signal_handler(int sig) {
    (void)sig;
    interrupted = 1;
    printf("\n\nOperation interrupted by careless user.\n");
    exit(1);
}

void print_usage(const char *program_name) {
    printf("Usage: %s [options]\n", program_name);
    printf("Options:\n");
    printf("  -h, --help     Show this help message\n");
    printf("  -l, --list     List available backups\n");
    printf("  -r, --restore  Restore from backup\n");
    printf("\n");
    printf("gitedit\n");
    printf("WARNING: This tool modifies Git objects directly and can break your repository.\n");
    printf("Always create backups before making changes.\n");
}

int restore_backup_interactive(void) {
    if (!list_backups()) {
        return 1;
    }
    
    printf("\nEnter backup timestamp to restore (or 0 to cancel): ");
    char backup_name[32];
    if (fgets(backup_name, sizeof(backup_name), stdin) == NULL) {
        return 1;
    }
    
    char *newline = strchr(backup_name, '\n');
    if (newline) *newline = '\0';
    
    if (strlen(backup_name) == 0 || strcmp(backup_name, "0") == 0) {
        printf("Restore cancelled.\n");
        return 1;
    }
    
    return restore_git_backup(backup_name) ? 0 : 1;
}

int main(int argc, char *argv[]) {
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            return 0;
        } else if (strcmp(argv[i], "-l") == 0 || strcmp(argv[i], "--list") == 0) {
            list_backups();
            return 0;
        } else if (strcmp(argv[i], "-r") == 0 || strcmp(argv[i], "--restore") == 0) {
            return restore_backup_interactive();
        } else {
            printf("Unknown option: %s\n", argv[i]);
            print_usage(argv[0]);
            return 1;
        }
    }
    
    if (!is_git_repository()) {
        printf("Error: Not in a Git repository.\n");
        printf("Please run this command from within a Git repository.\n");
        return 1;
    }
    
    char branch[256];
    if (!get_current_branch(branch, sizeof(branch))) {
        printf("Error: Could not determine current branch.\n");
        return 1;
    }
    
    printf("Current branch: %s\n", branch);
    
    printf("\nCreating safety backup...\n");
    if (!backup_git()) {
        printf("Error: Failed to create backup. Aborting for safety.\n");
        return 1;
    }
    
    printf("Backup created successfully.\n");
    
    commit_info_t commits[MAX_COMMITS];
    int commit_count = list_recent_commits(MAX_COMMITS, commits);
    
    if (commit_count <= 0) {
        printf("Error: No commits found on current branch.\n");
        return 1;
    }
    
    if (!tui_init()) {
        printf("Error: Failed to initialize TUI. Falling back to CLI mode.\n");
        int selection;
        if (!display_commits_and_select(commits, commit_count, &selection)) {
            printf("No commit selected. Exiting.\n");
            return 0;
        }
        
        return process_commit_edit(&commits[selection - 1], branch, argv[0]);
    }
    
    int selection;
    if (!tui_run_interface(commits, commit_count, &selection)) {
        tui_cleanup();
        printf("No commit selected. Exiting.\n");
        return 0;
    }
    
    int result = process_commit_edit(&commits[selection - 1], branch, argv[0]);
    tui_cleanup();
    return result;
}