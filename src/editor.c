#include "../include/editor.h"
#include "../include/utils.h"
#include "../include/git_objects.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int display_commits_and_select(commit_info_t *commits, int count, int *selection) {
    if (count <= 0) {
        printf("No commits found.\n");
        return 0;
    }
    
    printf("\nRecent commits:\n");
    printf("================================================================================\n");
    printf("  # | SHA (abbrev) | Date            | Author               | Message\n");
    printf("--------------------------------------------------------------------------------\n");
    
    for (int i = 0; i < count; i++) {
        char abbrev_sha[9];
        strncpy(abbrev_sha, commits[i].sha, 8);
        abbrev_sha[8] = '\0';
        
        printf("%2d | %-12s | %-15s | %-20s | %s\n", 
               i + 1, abbrev_sha, commits[i].date, commits[i].author, commits[i].message);
    }
    
    printf("================================================================================\n");
    
    while (1) {
        printf("\nSelect commit to edit (1-%d), or 0 to cancel: ", count);
        
        if (scanf("%d", selection) != 1) {
            while (getchar() != '\n');
            printf("Invalid input. Please enter a number.\n");
            continue;
        }
        
        if (*selection == 0) {
            printf("Cancelled.\n");
            return 0;
        }
        
        if (*selection < 1 || *selection > count) {
            printf("Invalid selection. Please enter a number between 1 and %d.\n", count);
            continue;
        }
        
        if (is_tip_commit(commits[*selection - 1].sha)) {
            printf("\nNOTE: This is the tip commit of the current branch.\n");
            printf("Editing this commit is generally safe.\n");
        } else {
            printf("\nWARNING: This is NOT the tip commit of the current branch.\n");
            printf("Editing this commit will change Git history and may cause issues for collaborators.\n");
            printf("Consider using 'git rebase -i' instead for safer history editing.\n");
        }
        
        printf("\nCommit details:\n");
        printf("SHA: %s\n", commits[*selection - 1].sha);
        printf("Author: %s\n", commits[*selection - 1].author);
        printf("Date: %s\n", commits[*selection - 1].date);
        
        char *full_commit = read_commit_object(commits[*selection - 1].sha);
        if (full_commit) {
            const char *msg_start = strstr(full_commit, "\n\n");
            if (msg_start) {
                printf("\nFull commit message:\n");
                printf("--------------------------------------------------------------------------------\n");
                printf("%s", msg_start + 2);
                printf("--------------------------------------------------------------------------------\n");
            }
            free(full_commit);
        }
        
        if (!confirm_action("Do you want to edit this commit message?")) {
            printf("Selection cancelled.\n");
            continue;
        }
        
        return 1;
    }
}

char* edit_commit_message(const char *current_message) {
    printf("\nCurrent commit message:\n");
    printf("--------------------------------------------------------------------------------\n");
    printf("%s\n", current_message);
    printf("--------------------------------------------------------------------------------\n");
    
    printf("\nEnter new commit message:\n");
    printf("Instructions:\n");
    printf("- Type your message line by line\n");
    printf("- End with a single line containing just a dot '.'\n");
    printf("- The dot line will not be included in the final message\n\n");
    
    size_t buffer_size = 4096;
    char *new_message = malloc(buffer_size);
    if (!new_message) return NULL;
    
    new_message[0] = '\0';
    char line[512];
    
    while (fgets(line, sizeof(line), stdin)) {
        if (strcmp(line, ".\n") == 0) {
            break;
        }
        
        size_t current_len = strlen(new_message);
        size_t line_len = strlen(line);
        
        if (current_len + line_len + 1 >= buffer_size) {
            buffer_size *= 2;
            char *new_buffer = realloc(new_message, buffer_size);
            if (!new_buffer) {
                free(new_message);
                return NULL;
            }
            new_message = new_buffer;
        }
        
        strcat(new_message, line);
    }
    
    size_t len = strlen(new_message);
    if (len > 0 && new_message[len - 1] == '\n') {
        new_message[len - 1] = '\0';
    }
    
    return new_message;
}

int confirm_action(const char *message) {
    char response[10];
    
    while (1) {
        printf("\n%s (Y/n): ", message);
        
        if (fgets(response, sizeof(response), stdin) == NULL) {
            return 0;
        }
        
        char *newline = strchr(response, '\n');
        if (newline) *newline = '\0';
        
        if (strlen(response) == 0) {
            return 1;
        }
        
        char c = response[0];
        if (c == 'y' || c == 'Y') {
            return 1;
        } else if (c == 'n' || c == 'N') {
            return 0;
        } else {
            printf("Please enter 'y' for yes or 'n' for no.\n");
        }
    }
}
