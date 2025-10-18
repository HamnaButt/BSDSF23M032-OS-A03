#include "shell.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* Trim whitespace from both ends (helper) */
static char *trim(char *s) {
    if (!s) return NULL;
    while(isspace((unsigned char)*s)) s++;
    if (*s == 0) return s;
    char *end = s + strlen(s) - 1;
    while(end > s && isspace((unsigned char)*end)) end--;
    end[1] = '\0';
    return s;
}

int main()
{
    char *cmdline_raw;
    char **arglist;
    int result;

    while (1) {
        // Reap any finished background jobs before prompt
        reap_jobs();

        // Read a full line (may contain ';' to chain)
        cmdline_raw = read_cmd("myshell> ", stdin);
        if (cmdline_raw == NULL) {
            printf("\nExiting shell...\n");
            break;
        }

        // If empty input, continue
        if (strlen(cmdline_raw) == 0) {
            free(cmdline_raw);
            continue;
        }

        // Split by semicolon ; into subcommands
        char *saveptr = NULL;
        char *segment = strtok_r(cmdline_raw, ";", &saveptr);
        while (segment != NULL) {
            char *seg_trim = trim(segment);
            if (seg_trim == NULL || strlen(seg_trim) == 0) {
                segment = strtok_r(NULL, ";", &saveptr);
                continue;
            }

            // Check for background operator '&' at the end
            int background = 0;
            size_t len = strlen(seg_trim);
            if (len > 0 && seg_trim[len - 1] == '&') {
                background = 1;
                // remove trailing &
                seg_trim[len - 1] = '\0';
                seg_trim = trim(seg_trim);
            }

            // Tokenize segment into arglist
            arglist = tokenize(seg_trim);
            if (arglist == NULL) {
                segment = strtok_r(NULL, ";", &saveptr);
                continue;
            }

            // Built-in handling
            if (!handle_builtin(arglist)) {
                // Execute; returns pid if background, 0 if foreground
                int pid_or_zero = execute(arglist, background);
                if (background && pid_or_zero > 0) {
                    // Add job using the full command string (seg_trim)
                    add_job((pid_t)pid_or_zero, seg_trim);
                    printf("[started] PID %d  %s\n", pid_or_zero, seg_trim);
                }
            }

            // free arglist
            for (int i = 0; arglist[i] != NULL; i++) free(arglist[i]);
            free(arglist);

            segment = strtok_r(NULL, ";", &saveptr);
        }

        free(cmdline_raw);
    }

    return 0;
}

