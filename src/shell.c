#include "shell.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <sys/wait.h>
#include <ctype.h>

/* ========== Job table ========== */
#define MAX_JOBS 128
typedef struct {
    pid_t pid;
    char *cmd;
} job_t;

static job_t jobs[MAX_JOBS];
static int job_count = 0;

/* Add job */
void add_job(pid_t pid, const char *cmd) {
    if (job_count >= MAX_JOBS) return;
    jobs[job_count].pid = pid;
    jobs[job_count].cmd = strdup(cmd);
    job_count++;
}

/* Remove job (by pid) */
void remove_job(pid_t pid) {
    int idx = -1;
    for (int i = 0; i < job_count; i++) {
        if (jobs[i].pid == pid) { idx = i; break; }
    }
    if (idx == -1) return;
    free(jobs[idx].cmd);
    for (int j = idx + 1; j < job_count; j++) jobs[j - 1] = jobs[j];
    job_count--;
}

/* List jobs */
static void list_jobs(void) {
    for (int i = 0; i < job_count; i++) {
        printf("[%d] PID: %d  %s\n", i + 1, jobs[i].pid, jobs[i].cmd);
    }
}

/* Reap finished background jobs without blocking */
void reap_jobs(void) {
    int status;
    pid_t pid;
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        remove_job(pid);
    }
}

/* ==========================================================
 * read_cmd() — uses GNU Readline
 * ========================================================== */
char* read_cmd(char* prompt, FILE* fp)
{
    (void)fp;
    char *cmdline = readline(prompt);
    if (cmdline == NULL) return NULL;
    if (strlen(cmdline) == 0) { free(cmdline); return strdup(""); }
    add_history(cmdline);
    return cmdline;
}

/* ==========================================================
 * tokenize() — splits command line into argv tokens
 * Handles quotes properly: echo "YES branch" -> ["echo","YES branch"]
 * ========================================================== */
char** tokenize(char* cmdline)
{
    if (cmdline == NULL || cmdline[0] == '\0' || cmdline[0] == '\n')
        return NULL;

    char **arglist = malloc(sizeof(char*) * (MAXARGS + 1));
    if (!arglist) return NULL;

    int argnum = 0;
    char *p = cmdline;

    while (*p != '\0') {
        while (isspace((unsigned char)*p)) p++;
        if (*p == '\0') break;

        char token[ARGLEN] = {0};
        int ti = 0;

        if (*p == '"' || *p == '\'') {
            char quote = *p++;
            while (*p && *p != quote && ti < ARGLEN - 1) {
                token[ti++] = *p++;
            }
            if (*p == quote) p++; // skip closing quote
        } else {
            while (*p && !isspace((unsigned char)*p) && ti < ARGLEN - 1) {
                token[ti++] = *p++;
            }
        }
        token[ti] = '\0';
        arglist[argnum++] = strdup(token);
        if (argnum >= MAXARGS) break;
    }

    arglist[argnum] = NULL;
    return arglist;
}

/* ==========================================================
 * Variable store (simple linked list)
 * ========================================================== */
typedef struct var {
    char *name;
    char *value;
    struct var *next;
} var_t;
static var_t *var_head = NULL;

void set_var(const char *name, const char *value) {
    if (!name) return;
    var_t *p = var_head;
    while (p) {
        if (strcmp(p->name, name) == 0) {
            free(p->value);
            p->value = strdup(value ? value : "");
            return;
        }
        p = p->next;
    }
    var_t *n = malloc(sizeof(var_t));
    n->name = strdup(name);
    n->value = strdup(value ? value : "");
    n->next = var_head;
    var_head = n;
}

const char *get_var(const char *name) {
    var_t *p = var_head;
    while (p) {
        if (strcmp(p->name, name) == 0) return p->value;
        p = p->next;
    }
    return NULL;
}

void print_all_vars(void) {
    var_t *p = var_head;
    while (p) {
        printf("%s=%s\n", p->name, p->value);
        p = p->next;
    }
}

void free_vars(void) {
    var_t *p = var_head;
    while (p) {
        var_t *n = p->next;
        free(p->name);
        free(p->value);
        free(p);
        p = n;
    }
    var_head = NULL;
}

/* ==========================================================
 * Expand $VARNAME inside a token
 * ========================================================== */
static char *expand_string(const char *orig) {
    if (!orig) return NULL;
    size_t len = strlen(orig);
    if (strchr(orig, '$') == NULL) return strdup(orig);

    char buf[ARGLEN * 4];
    buf[0] = '\0';
    size_t bi = 0;

    for (size_t i = 0; i < len; ) {
        if (orig[i] == '$') {
            i++;
            if (i >= len) break;
            char name[ARGLEN];
            int ni = 0;
            while (i < len && (isalnum((unsigned char)orig[i]) || orig[i] == '_')) {
                if (ni < ARGLEN - 1) name[ni++] = orig[i];
                i++;
            }
            name[ni] = '\0';
            const char *val = get_var(name);
            if (val) strncat(buf, val, sizeof(buf) - strlen(buf) - 1);
        } else {
            size_t remain = sizeof(buf) - bi - 1;
            if (remain > 0) {
                buf[bi++] = orig[i++];
                buf[bi] = '\0';
            } else break;
        }
    }
    return strdup(buf);
}

/* Expand variables for an arglist */
static void expand_args_inplace(char **args) {
    for (int i = 0; args[i] != NULL; i++) {
        char *expanded = expand_string(args[i]);
        free(args[i]);
        args[i] = strdup(expanded);
        free(expanded);
    }
}

/* ==========================================================
 * handle_builtin() — built-in commands
 * ========================================================== */
int handle_builtin(char **args)
{
    if (args == NULL || args[0] == NULL) return 1;

    if (strcmp(args[0], "exit") == 0) {
        printf("Exiting shell...\n");
        free_vars();
        exit(0);
    }
    else if (strcmp(args[0], "cd") == 0) {
        if (args[1] == NULL) fprintf(stderr, "cd: expected argument\n");
        else if (chdir(args[1]) != 0) perror("cd");
        return 1;
    }
    else if (strcmp(args[0], "help") == 0) {
        printf("Built-in commands:\n");
        printf("  cd <dir>\n  help\n  exit\n  jobs\n  history\n  set\n");
        printf("Supports: VAR=value, $VAR expansion, if..then..else..fi\n");
        return 1;
    }
    else if (strcmp(args[0], "jobs") == 0) {
        list_jobs();
        return 1;
    }
    else if (strcmp(args[0], "history") == 0) {
        HIST_ENTRY **the_history = history_list();
        if (the_history) {
            for (int i = 0; the_history[i]; i++)
                printf("%d  %s\n", i + 1, the_history[i]->line);
        }
        return 1;
    }
    else if (strcmp(args[0], "set") == 0) {
        print_all_vars();
        return 1;
    }

    return 0; // not built-in
}

/* ==========================================================
 * run_command_line() — tokenizes, expands, executes
 * ========================================================== */
int run_command_line(char *rawline, int background)
{
    if (!rawline) return -1;

    char *dup = strdup(rawline);
    char **arglist = tokenize(dup);
    free(dup);
    if (arglist == NULL) return -1;

    // assignment handling: VAR=value
    char *first = arglist[0];
    char *eq = strchr(first, '=');
    if (eq != NULL) {
        size_t name_len = eq - first;
        if (name_len > 0 && name_len < ARGLEN) {
            char name[ARGLEN];
            strncpy(name, first, name_len);
            name[name_len] = '\0';
            const char *val = eq + 1;
            if (val[0] == '"' && val[strlen(val)-1] == '"' && strlen(val) >= 2) {
                char *tmp = strndup(val + 1, strlen(val) - 2);
                set_var(name, tmp);
                free(tmp);
            } else {
                set_var(name, val);
            }
            for (int i = 0; arglist[i]; i++) free(arglist[i]);
            free(arglist);
            return 0;
        }
    }

    expand_args_inplace(arglist);

    if (handle_builtin(arglist)) {
        for (int i = 0; arglist[i]; i++) free(arglist[i]);
        free(arglist);
        return 0;
    }

    int result = execute(arglist, background);

    for (int i = 0; arglist[i]; i++) free(arglist[i]);
    free(arglist);
    return result;
}
