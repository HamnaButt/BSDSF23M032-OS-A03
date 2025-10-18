#include "shell.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <sys/wait.h>

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
    // keep reaping while there are finished children
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        // remove from job list if present
        remove_job(pid);
        // Optional: print done message
        // printf("[reaped] PID %d\n", pid);
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
 * tokenize() — splits a single command into argv tokens
 * ========================================================== */
char** tokenize(char* cmdline)
{
    if (cmdline == NULL || cmdline[0] == '\0' || cmdline[0] == '\n')
        return NULL;

    char **arglist = (char**)malloc(sizeof(char*) * (MAXARGS + 1));
    for (int i = 0; i < MAXARGS + 1; i++) {
        arglist[i] = (char*)malloc(sizeof(char) * ARGLEN);
        bzero(arglist[i], ARGLEN);
    }

    char *cp = cmdline;
    char *start;
    int len;
    int argnum = 0;

    while (*cp != '\0' && argnum < MAXARGS) {
        while (*cp == ' ' || *cp == '\t') cp++;
        if (*cp == '\0') break;
        start = cp;
        len = 1;
        while (*++cp != '\0' && !(*cp == ' ' || *cp == '\t')) len++;
        strncpy(arglist[argnum], start, len);
        arglist[argnum][len] = '\0';
        argnum++;
    }

    if (argnum == 0) {
        for (int i = 0; i < MAXARGS + 1; i++) free(arglist[i]);
        free(arglist);
        return NULL;
    }

    arglist[argnum] = NULL;
    return arglist;
}

/* ==========================================================
 * handle_builtin() — built-in shell commands including jobs
 * ========================================================== */
int handle_builtin(char **args)
{
    if (args == NULL || args[0] == NULL) return 1;

    if (strcmp(args[0], "exit") == 0) {
        printf("Exiting shell...\n");
        exit(0);
    }
    else if (strcmp(args[0], "cd") == 0) {
        if (args[1] == NULL) fprintf(stderr, "cd: expected argument\n");
        else if (chdir(args[1]) != 0) perror("cd");
        return 1;
    }
    else if (strcmp(args[0], "help") == 0) {
        printf("Built-in commands:\n");
        printf("  cd <dir>\n  help\n  exit\n  jobs\n");
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

    return 0; // not built-in
}

