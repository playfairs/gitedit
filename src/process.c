#include "../include/utils.h"
#include "../include/backup.h"
#include "../include/git_objects.h"
#include "../include/tui.h"
#include "../include/editor.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int process_commit_edit(commit_info_t *selected_commit, const char *branch, const char *program_name) {
    char *commit_data = read_commit_object(selected_commit->sha);
    if (!commit_data) {
        if (tui_init()) {
            tui_show_error("Failed to read commit object");
            tui_cleanup();
        } else {
            printf("Error: Failed to read commit object.\n");
        }
        return 1;
    }
    
    char current_message[2048];
    extract_commit_message(commit_data, current_message, sizeof(current_message));
    
    char *new_message;
    if (tui_init()) {
        new_message = tui_edit_message(current_message);
        if (!new_message) {
            tui_cleanup();
            free(commit_data);
            return 0;
        }
        
        if (strcmp(current_message, new_message) == 0) {
            tui_show_success("Message unchanged. No update needed.");
            tui_cleanup();
            free(commit_data);
            free(new_message);
            return 0;
        }
        
        if (!tui_confirm_dialog("Apply these changes?")) {
            tui_cleanup();
            free(commit_data);
            free(new_message);
            return 0;
        }
    } else {
        new_message = edit_commit_message(current_message);
        if (!new_message) {
            printf("Error: Failed to get new message.\n");
            free(commit_data);
            return 1;
        }
        
        if (strcmp(current_message, new_message) == 0) {
            printf("Message unchanged. No update needed.\n");
            free(commit_data);
            free(new_message);
            return 0;
        }
        
        printf("\nPreview of changes:\n");
        printf("Old message:\n");
        printf("--------------------------------------------------------------------------------\n");
        printf("%s\n", current_message);
        printf("--------------------------------------------------------------------------------\n");
        printf("New message:\n");
        printf("--------------------------------------------------------------------------------\n");
        printf("%s\n", new_message);
        printf("--------------------------------------------------------------------------------\n");
        
        if (!confirm_action("Apply these changes?")) {
            printf("Changes cancelled.\n");
            free(commit_data);
            free(new_message);
            return 0;
        }
    }
    
    char *new_commit_data = NULL;
    
    const char *double_newline = strstr(commit_data, "\n\n");
    if (double_newline) {
        size_t header_len = double_newline - commit_data + 2;
        size_t new_message_len = strlen(new_message);
        
        new_commit_data = malloc(header_len + new_message_len + 1);
        if (new_commit_data) {
            memcpy(new_commit_data, commit_data, header_len);
            strcpy(new_commit_data + header_len, new_message);
        }
    } else {
        const char *single_newline = strstr(commit_data, "\n");
        if (single_newline) {
            size_t header_len = single_newline - commit_data + 1;
            size_t new_message_len = strlen(new_message);
            
            new_commit_data = malloc(header_len + new_message_len + 2);
            if (new_commit_data) {
                memcpy(new_commit_data, commit_data, header_len);
                new_commit_data[header_len] = '\n';
                strcpy(new_commit_data + header_len + 1, new_message);
            }
        } else {
            size_t header_len = strlen(commit_data);
            size_t new_message_len = strlen(new_message);
            
            new_commit_data = malloc(header_len + 2 + new_message_len + 1);
            if (new_commit_data) {
                strcpy(new_commit_data, commit_data);
                strcat(new_commit_data, "\n\n");
                strcat(new_commit_data, new_message);
            }
        }
    }
    
    if (!new_commit_data) {
        if (tui_init()) {
            tui_show_error("Failed to rebuild commit object");
            tui_cleanup();
        } else {
            printf("Error: Failed to rebuild commit object.\n");
        }
        free(commit_data);
        free(new_message);
        return 1;
    }
    
    char new_sha[SHA1_HEX_SIZE];
    calculate_sha1(new_commit_data, strlen(new_commit_data), new_sha);
    
    if (!write_commit_object(new_sha, new_commit_data, strlen(new_commit_data))) {
        if (tui_init()) {
            tui_show_error("Failed to write new commit object");
            tui_cleanup();
        } else {
            printf("Error: Failed to write new commit object.\n");
        }
        free(commit_data);
        free(new_message);
        free(new_commit_data);
        return 1;
    }
    
    if (is_tip_commit(selected_commit->sha)) {
        update_branch_reference(branch, new_sha);
        if (tui_init()) {
            tui_show_success("Commit message updated successfully!");
            tui_cleanup();
        } else {
            printf("Branch reference updated.\n");
        }
    } else {
        if (tui_init()) {
            tui_show_error("This was not a tip commit. You may need to use 'git reset' to update your branch.");
            tui_cleanup();
        } else {
            printf("Note: This was not a tip commit. You may need to use 'git reset' or similar\n");
            printf("commands to update your branch reference to point to the new commit.\n");
        }
    }
    
    char log_details[512];
    snprintf(log_details, sizeof(log_details), 
             "Edited commit %s -> %s on branch %s", 
             selected_commit->sha, new_sha, branch);
    log_action("COMMIT_EDITED", log_details);
    
    if (!tui_init()) {
        printf("\nCommit message updated successfully!\n");
        printf("Old SHA: %s\n", selected_commit->sha);
        printf("New SHA: %s\n", new_sha);
        printf("\nA backup of your original .git directory has been created.\n");
        printf("Use '%s --list' to see available backups.\n", program_name);
        printf("Use '%s --restore' to restore from backup if needed.\n", program_name);
    }
    
    free(commit_data);
    free(new_message);
    free(new_commit_data);
    
    return 0;
}
