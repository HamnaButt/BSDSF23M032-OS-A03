#include "shell.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* Trim spaces */
static char *trim(char *s) {
    if (!s) return NULL;
    while (isspace((unsigned char)*s)) s++;
    if (*s == 0) return s;
    char *end = s + strlen(s) - 1;
    while (end > s && isspace((unsigned char)*end)) end--;
    end[1] = '\0';
    return s;
}

/* Get exit status of a single command line */
static int run_and_get_status(char *line) {
    if (!line || strlen(line) == 0) return 1;
    int result = run_command_line(line, 0);
    if (result < 0) return 1;
    if (result > 0) return 0;  // background success

    char *dup = strdup(line);
    char **args = tokenize(dup);
    free(dup);
    if (!args) return 1;
    int status = execute_get_status(args);
    for (int i = 0; args[i]; i++) free(args[i]);
    free(args);
    return status;
}

int main(void) {
    char *cmdline_raw;

    while (1) {
        reap_jobs();
        cmdline_raw = read_cmd("myshell> ", stdin);
        if (!cmdline_raw) {
            printf("\nExiting shell...\n");
            break;
        }
        if (strlen(cmdline_raw) == 0) {
            free(cmdline_raw);
            continue;
        }

        char *saveptr = NULL;
        char *segment = strtok_r(cmdline_raw, ";", &saveptr);

        while (segment) {
            char *seg_trim = trim(segment);
            if (!seg_trim || strlen(seg_trim) == 0) {
                segment = strtok_r(NULL, ";", &saveptr);
                continue;
            }

            /* ---------- IF-THEN-ELSE-FI ---------- */
            if (strncmp(seg_trim, "if", 2) == 0 &&
                (seg_trim[2] == ' ' || seg_trim[2] == '\0')) {

                char *cond = strdup(trim(seg_trim + 2));
                char *line;
                char *then_cmds[256];
                char *else_cmds[256];
                int then_count = 0, else_count = 0;
                int in_else = 0;

                while ((line = read_cmd("", stdin)) != NULL) {
                    char *t = trim(line);
                    if (strcasecmp(t, "then") == 0) {
                        // skip, just marker
                    } else if (strcasecmp(t, "else") == 0) {
                        in_else = 1;
                    } else if (strcasecmp(t, "fi") == 0) {
                        free(line);
                        break; // end of block
                    } else {
                        if (!in_else)
                            then_cmds[then_count++] = strdup(t);
                        else
                            else_cmds[else_count++] = strdup(t);
                    }
                    free(line);
                }

                int cond_status = run_and_get_status(cond);

                if (cond_status == 0) {
                    for (int i = 0; i < then_count; i++) {
                        run_command_line(then_cmds[i], 0);
                        free(then_cmds[i]);
                    }
                } else {
                    for (int i = 0; i < else_count; i++) {
                        run_command_line(else_cmds[i], 0);
                        free(else_cmds[i]);
                    }
                }

                free(cond);
                segment = strtok_r(NULL, ";", &saveptr);
                continue;
            }

            /* ---------- Normal command ---------- */
            int background = 0;
            size_t len = strlen(seg_trim);
            if (len > 0 && seg_trim[len - 1] == '&') {
                background = 1;
                seg_trim[len - 1] = '\0';
                seg_trim = trim(seg_trim);
            }

            int rv = run_command_line(seg_trim, background);
            if (background && rv > 0) {
                add_job((pid_t)rv, seg_trim);
                printf("[started] PID %d  %s\n", rv, seg_trim);
            }

            segment = strtok_r(NULL, ";", &saveptr);
        }

        free(cmdline_raw);
    }

    return 0;
}
