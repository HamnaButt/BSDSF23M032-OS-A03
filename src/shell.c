#include "shell.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <sys/wait.h>   // <-- add this line
#include <unistd.h>     // already included

/* ================= Job table ================= */
#define MAX_JOBS 128
typedef struct {
    pid_t pid;
    char *cmd;
} job_t;

static job_t jobs[MAX_JOBS];
static int job_count = 0;

void add_job(pid_t pid, const char *cmd) {
    if (job_count >= MAX_JOBS) return;
    jobs[job_count].pid = pid;
    jobs[job_count].cmd = strdup(cmd);
    job_count++;
}

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

void reap_jobs(void) {
    int status;
    pid_t pid;
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        remove_job(pid);
    }
}

/* ================= Read command ================= */
char* read_cmd(char* prompt, FILE* fp) {
    (void)fp;
    char *cmdline = readline(prompt);
    if (!cmdline) return NULL;
    if (strlen(cmdline) == 0) { free(cmdline); return strdup(""); }
    add_history(cmdline);
    return cmdline;
}

/* ================= Tokenize ================= */
char** tokenize(char* cmdline) {
    if (!cmdline || !*cmdline) return NULL;

    char **arglist = malloc(sizeof(char*) * (MAXARGS + 1));
    if (!arglist) return NULL;

    int argnum = 0;
    char *p = cmdline;

    while (*p) {
        while (isspace((unsigned char)*p)) p++;
        if (!*p) break;

        char token[ARGLEN] = {0};
        int ti = 0;

        if (*p == '"' || *p == '\'') {
            char quote = *p++;
            while (*p && *p != quote && ti < ARGLEN - 1) token[ti++] = *p++;
            if (*p == quote) p++;
        } else {
            while (*p && !isspace((unsigned char)*p) && ti < ARGLEN - 1) token[ti++] = *p++;
        }

        token[ti] = '\0';
        arglist[argnum++] = strdup(token);
        if (argnum >= MAXARGS) break;
    }
    arglist[argnum] = NULL;
    return arglist;
}

/* ================= Variable store ================= */
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

/* ================= run_command_line ================= */
int run_command_line(char *rawline, int background) {
    if (!rawline || !*rawline) return 0;
    char **args = tokenize(rawline);
    if (!args) return 0;
    extern int execute(char **arglist, int background);  // defined in execute.c
    int status = execute(args, background);

    for (int i = 0; args[i]; i++) free(args[i]);
    free(args);
    return status;
}
