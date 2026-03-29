#include "../include/tui.h"
#include "../include/utils.h"
#include <ncurses.h>
#include <string.h>
#include <stdlib.h>

static WINDOW *main_win;
static WINDOW *commit_win;
static WINDOW *status_win;

int tui_init(void) {
    main_win = initscr();
    if (!main_win) return 0;
    
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);
    start_color();
    use_default_colors();
    
    // Use terminal default colors, no backgrounds
    init_pair(1, COLOR_CYAN, -1);    // Cyan for headers
    init_pair(2, COLOR_WHITE, -1);   // White for normal text
    init_pair(3, COLOR_RED, -1);     // Red for errors
    init_pair(4, COLOR_GREEN, -1);   // Green for success
    init_pair(5, COLOR_YELLOW, -1);  // Yellow for highlights
    init_pair(6, COLOR_BLUE, -1);    // Blue for selection
    
    refresh();
    return 1;
}

void tui_cleanup(void) {
    if (main_win) {
        endwin();
        main_win = NULL;
    }
}

void tui_show_error(const char *message) {
    int height = 5, width = strlen(message) + 4;
    int starty = (LINES - height) / 2;
    int startx = (COLS - width) / 2;
    
    WINDOW *error_win = newwin(height, width, starty, startx);
    wbkgd(error_win, COLOR_PAIR(3));
    box(error_win, 0, 0);
    mvwprintw(error_win, 2, 2, "%s", message);
    wrefresh(error_win);
    
    wgetch(error_win);
    delwin(error_win);
    touchwin(stdscr);
    refresh();
}

void tui_show_success(const char *message) {
    int height = 5, width = strlen(message) + 4;
    int starty = (LINES - height) / 2;
    int startx = (COLS - width) / 2;
    
    WINDOW *success_win = newwin(height, width, starty, startx);
    wbkgd(success_win, COLOR_PAIR(4));
    box(success_win, 0, 0);
    mvwprintw(success_win, 2, 2, "%s", message);
    wrefresh(success_win);
    
    wgetch(success_win);
    delwin(success_win);
    touchwin(stdscr);
    refresh();
}

int tui_confirm_dialog(const char *message) {
    int height = 7, width = strlen(message) + 10;
    int starty = (LINES - height) / 2;
    int startx = (COLS - width) / 2;
    
    WINDOW *dialog_win = newwin(height, width, starty, startx);
    wbkgd(dialog_win, COLOR_PAIR(2));
    box(dialog_win, 0, 0);
    mvwprintw(dialog_win, 2, 2, "%s", message);
    mvwprintw(dialog_win, 4, 2, "[Y] Yes  [N] No");
    wrefresh(dialog_win);
    
    int choice = 0;
    while (choice == 0) {
        int ch = wgetch(dialog_win);
        switch (ch) {
            case 'y':
            case 'Y':
            case '\n':
                choice = 1;
                break;
            case 'n':
            case 'N':
            case 27:
                choice = 0;
                break;
        }
    }
    
    delwin(dialog_win);
    touchwin(stdscr);
    refresh();
    return choice;
}

char* tui_edit_message(const char *current_message) {
    int height = LINES - 10;
    int width = COLS - 10;
    int starty = 5;
    int startx = 5;
    
    WINDOW *edit_win = newwin(height, width, starty, startx);
    wbkgd(edit_win, COLOR_PAIR(2));
    box(edit_win, 0, 0);
    mvwprintw(edit_win, 0, 2, " Edit Commit Message ");
    
    mvwprintw(edit_win, 2, 2, "Current message:");
    mvwprintw(edit_win, 3, 2, "%.*s", width - 4, current_message);
    mvwprintw(edit_win, height - 4, 2, "New message (ESC to cancel, Ctrl+O to save):");
    
    char *new_message = malloc(2048);
    if (!new_message) {
        delwin(edit_win);
        return NULL;
    }
    
    new_message[0] = '\0';
    int pos = 0;
    int line = height - 3;
    int col = 2;
    
    curs_set(1);
    wmove(edit_win, line, col);
    wrefresh(edit_win);
    
    int editing = 1;
    while (editing) {
        int ch = wgetch(edit_win);
        
        switch (ch) {
            case 27: // ESC
                free(new_message);
                new_message = NULL;
                editing = 0;
                break;
                
            case 15: // Ctrl+O (save)
                editing = 0;
                break;
                
            case 127: // Backspace
            case KEY_BACKSPACE:
                if (pos > 0) {
                    pos--;
                    new_message[pos] = '\0';
                    if (col > 2) {
                        col--;
                    } else if (line > height - 3) {
                        line--;
                        col = width - 3;
                    }
                }
                break;
                
            case '\n':
                if (pos < 2047) {
                    new_message[pos++] = '\n';
                    if (line < height - 2) {
                        line++;
                        col = 2;
                    }
                }
                break;
                
            default:
                if (ch >= 32 && ch <= 126 && pos < 2047) {
                    new_message[pos++] = ch;
                    new_message[pos] = '\0';
                    if (col < width - 3) {
                        col++;
                    } else if (line < height - 2) {
                        line++;
                        col = 2;
                    }
                }
                break;
        }
        
        wclear(edit_win);
        wbkgd(edit_win, COLOR_PAIR(2));
        box(edit_win, 0, 0);
        mvwprintw(edit_win, 0, 2, " Edit Commit Message ");
        mvwprintw(edit_win, 2, 2, "Current:");
        mvwprintw(edit_win, 3, 2, "%.*s", width - 4, current_message);
        mvwprintw(edit_win, height - 4, 2, "New message (ESC=cancel, Ctrl+O=save):");
        mvwprintw(edit_win, height - 3, 2, "%.*s", width - 4, new_message);
        wmove(edit_win, line, col);
        wrefresh(edit_win);
    }
    
    curs_set(0);
    delwin(edit_win);
    touchwin(stdscr);
    refresh();
    
    return new_message;
}

int tui_run_interface(commit_info_t *commits, int count, int *selection) {
    int height = LINES - 8;
    int width = COLS - 4;
    int starty = 4;
    int startx = 2;
    
    clear();
    
    // Draw main border
    attron(COLOR_PAIR(1));
    box(stdscr, 0, 0);
    attroff(COLOR_PAIR(1));
    
    // Header
    attron(COLOR_PAIR(1) | A_BOLD);
    mvprintw(1, 2, " gitedit - Git Commit Message Editor ");
    mvprintw(2, 2, " j/k:↓↑  h/l:home/end  gg/G:top/bottom  o:edit  q:quit  ?:help ");
    attroff(COLOR_PAIR(1) | A_BOLD);
    
    // Create commit window with border
    commit_win = newwin(height, width, starty, startx);
    wbkgd(commit_win, COLOR_PAIR(2));
    
    // Status bar
    status_win = newwin(3, COLS - 4, LINES - 4, 2);
    wbkgd(status_win, COLOR_PAIR(5));
    
    int selected = 0;
    int running = 1;
    int top = 0;
    
    while (running) {
        wclear(commit_win);
        box(commit_win, 0, 0);
        
        // Window title
        wattron(commit_win, COLOR_PAIR(1) | A_BOLD);
        mvwprintw(commit_win, 0, 2, " Recent Commits ");
        wattroff(commit_win, COLOR_PAIR(1) | A_BOLD);
        
        int display_count = height - 2;
        for (int i = 0; i < display_count && (top + i) < count; i++) {
            int actual_index = top + i;
            int y = i + 1;
            
            char abbrev_sha[9];
            strncpy(abbrev_sha, commits[actual_index].sha, 8);
            abbrev_sha[8] = '\0';
            
            // Selection highlighting
            if (actual_index == selected) {
                wattron(commit_win, COLOR_PAIR(6) | A_REVERSE | A_BOLD);
            } else {
                wattron(commit_win, COLOR_PAIR(2));
            }
            
            // Truncate message if too long
            char msg_display[60];
            strncpy(msg_display, commits[actual_index].message, 59);
            msg_display[59] = '\0';
            
            mvwprintw(commit_win, y, 2, "%2d | %-8s | %-15s | %-20s | %s", 
                     actual_index + 1, abbrev_sha, commits[actual_index].date, 
                     commits[actual_index].author, msg_display);
            
            if (actual_index == selected) {
                wattroff(commit_win, COLOR_PAIR(6) | A_REVERSE | A_BOLD);
            } else {
                wattroff(commit_win, COLOR_PAIR(2));
            }
        }
        
        // Scroll indicators
        if (count > display_count) {
            if (top > 0) {
                mvwprintw(commit_win, 0, width - 2, "▲");
            }
            if (top + display_count < count) {
                mvwprintw(commit_win, height - 1, width - 2, "▼");
            }
        }
        
        wrefresh(commit_win);
        
        // Status bar
        wclear(status_win);
        box(status_win, 0, 0);
        if (selected < count) {
            char abbrev_sha[9];
            strncpy(abbrev_sha, commits[selected].sha, 8);
            abbrev_sha[8] = '\0';
            
            wattron(status_win, COLOR_PAIR(5) | A_BOLD);
            mvwprintw(status_win, 1, 2, " %s - %s ", abbrev_sha, commits[selected].message);
            wattroff(status_win, COLOR_PAIR(5) | A_BOLD);
            
            wattron(status_win, COLOR_PAIR(2));
            mvwprintw(status_win, 2, 2, " Commit %d/%d | Line %d | Press ? for help ", selected + 1, count, selected + 1);
            wattroff(status_win, COLOR_PAIR(2));
        }
        wrefresh(status_win);
        
        int ch = getch();
        switch (ch) {
            // Vim navigation
            case 'j':
                if (selected < count - 1) {
                    selected++;
                    if (selected >= top + display_count) {
                        top = selected - display_count + 1;
                    }
                }
                break;
                
            case 'k':
                if (selected > 0) {
                    selected--;
                    if (selected < top) {
                        top = selected;
                    }
                }
                break;
                
            case 'h':
            case KEY_LEFT:
                selected = 0;
                top = 0;
                break;
                
            case 'l':
            case KEY_RIGHT:
                selected = count - 1;
                top = (count > display_count) ? count - display_count : 0;
                break;
                
            case 'g':
                // Wait for second 'g' for gg
                if (getch() == 'g') {
                    selected = 0;
                    top = 0;
                }
                break;
                
            case 'G':
                selected = count - 1;
                top = (count > display_count) ? count - display_count : 0;
                break;
                
            case '0':
            case '^':
                selected = 0;
                top = 0;
                break;
                
            case '$':
                selected = count - 1;
                top = (count > display_count) ? count - display_count : 0;
                break;
                
            // Page navigation
            case KEY_PPAGE:
            case 'b':
            case 'B':
                selected = (selected > display_count) ? selected - display_count : 0;
                top = (top > display_count) ? top - display_count : 0;
                break;
                
            case KEY_NPAGE:
            case ' ':
            case 'f':
            case 'F':
                selected = (selected + display_count < count) ? selected + display_count : count - 1;
                if (selected >= top + display_count) {
                    top = selected - display_count + 1;
                }
                break;
                
            // Line numbers
            case '1': case '2': case '3': case '4': case '5':
            case '6': case '7': case '8': case '9':
                {
                    int num = ch - '0';
                    if (num <= count) {
                        selected = num - 1;
                        top = 0;
                        if (selected >= display_count) {
                            top = selected - display_count + 1;
                        }
                    }
                }
                break;
                
            // Quit
            case 'q':
            case ':':
                if (ch == ':' && getch() == 'q' && getch() == '\n') {
                    running = 0;
                    *selection = 0;
                } else if (ch == 'q') {
                    running = 0;
                    *selection = 0;
                }
                break;
                
            case 27: // ESC
                running = 0;
                *selection = 0;
                break;
                
            // Edit
            case '\n':
            case '\r':
            case 'o':
            case 'O':
            case 'e':
            case 'E':
            case 'i':
            case 'a':
                *selection = selected + 1;
                running = 0;
                break;
                
            // Help
            case '?':
            case KEY_F(1):
                {
                    int help_height = 15, help_width = 70;
                    int help_starty = (LINES - help_height) / 2;
                    int help_startx = (COLS - help_width) / 2;
                    
                    WINDOW *help_win = newwin(help_height, help_width, help_starty, help_startx);
                    wbkgd(help_win, COLOR_PAIR(2));
                    box(help_win, 0, 0);
                    
                    wattron(help_win, COLOR_PAIR(1) | A_BOLD);
                    mvwprintw(help_win, 0, 2, " Vim-Style Help ");
                    wattroff(help_win, COLOR_PAIR(1) | A_BOLD);
                    
                    wattron(help_win, COLOR_PAIR(2));
                    mvwprintw(help_win, 2, 2, "Navigation:");
                    mvwprintw(help_win, 3, 4, "j/k     - Move down/up");
                    mvwprintw(help_win, 4, 4, "h/l     - Home/End (first/last commit)");
                    mvwprintw(help_win, 5, 4, "gg/G    - Go to top/bottom");
                    mvwprintw(help_win, 6, 4, "0/^/$   - Go to top/middle/bottom");
                    mvwprintw(help_win, 7, 4, "b/f     - Page up/down");
                    mvwprintw(help_win, 8, 4, "1-9     - Jump to commit N");
                    mvwprintw(help_win, 9, 4, "jj      - Page down");
                    mvwprintw(help_win, 10, 2, "Actions:");
                    mvwprintw(help_win, 11, 4, "o/e/i/a/Enter - Edit selected commit");
                    mvwprintw(help_win, 12, 4, "q or :q - Quit");
                    mvwprintw(help_win, 13, 4, "ESC      - Quit");
                    mvwprintw(help_win, 14, 4, "? or F1  - This help");
                    wattroff(help_win, COLOR_PAIR(2));
                    
                    wrefresh(help_win);
                    wgetch(help_win);
                    delwin(help_win);
                    touchwin(stdscr);
                    refresh();
                }
                break;
        }
    }
    
    delwin(commit_win);
    delwin(status_win);
    
    return (*selection > 0);
}
