#include "shell.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// ==============================
// Global variables for history
// ==============================
#define HISTORY_SIZE 20
char *history[HISTORY_SIZE];
int hist_count = 0;

// ==============================
// Function: read_cmd()
// ==============================
char* read_cmd(char* prompt, FILE* fp) {
    printf("%s", prompt);
    char* cmdline = (char*) malloc(sizeof(char) * MAX_LEN);
    int c, pos = 0;

    while ((c = getc(fp)) != EOF) {
        if (c == '\n') break;
        cmdline[pos++] = c;
    }

    if (c == EOF && pos == 0) {
        free(cmdline);
        return NULL; // Handle Ctrl+D
    }

    cmdline[pos] = '\0';
    return cmdline;
}

// ==============================
// Function: tokenize()
// ==============================
char** tokenize(char* cmdline) {
    // Edge case: empty command line
    if (cmdline == NULL || cmdline[0] == '\0' || cmdline[0] == '\n') {
        return NULL;
    }

    char** arglist = (char**)malloc(sizeof(char*) * (MAXARGS + 1));
    for (int i = 0; i < MAXARGS + 1; i++) {
        arglist[i] = (char*)malloc(sizeof(char) * ARGLEN);
        bzero(arglist[i], ARGLEN);
    }

    char* cp = cmdline;
    char* start;
    int len;
    int argnum = 0;

    while (*cp != '\0' && argnum < MAXARGS) {
        while (*cp == ' ' || *cp == '\t') cp++; // Skip whitespace
        if (*cp == '\0') break;

        start = cp;
        len = 1;
        while (*++cp != '\0' && !(*cp == ' ' || *cp == '\t')) {
            len++;
        }
        strncpy(arglist[argnum], start, len);
        arglist[argnum][len] = '\0';
        argnum++;
    }

    if (argnum == 0) { // No arguments parsed
        for(int i = 0; i < MAXARGS + 1; i++) free(arglist[i]);
        free(arglist);
        return NULL;
    }

    arglist[argnum] = NULL;
    return arglist;
}

// ==============================
// Function: add_to_history()
// ==============================
void add_to_history(char *cmdline) {
    if (cmdline == NULL || strlen(cmdline) == 0)
        return;

    if (hist_count < HISTORY_SIZE) {
        history[hist_count++] = strdup(cmdline);
    } else {
        free(history[0]);
        for (int i = 1; i < HISTORY_SIZE; i++)
            history[i - 1] = history[i];
        history[HISTORY_SIZE - 1] = strdup(cmdline);
    }
}

// ==============================
// Function: handle_builtin()
// ==============================
int handle_builtin(char **args)
{
    if (args == NULL || args[0] == NULL)
        return 1;  // Empty command

    // exit command
    if (strcmp(args[0], "exit") == 0) {
        printf("Exiting shell...\n");
        exit(0);
    }

    // cd command
    else if (strcmp(args[0], "cd") == 0) {
        if (args[1] == NULL)
            fprintf(stderr, "cd: expected argument\n");
        else if (chdir(args[1]) != 0)
            perror("cd");
        return 1;
    }

    // help command
    else if (strcmp(args[0], "help") == 0) {
        printf("Built-in commands:\n");
        printf("  cd <dir>     - Change directory\n");
        printf("  help         - Show this help message\n");
        printf("  exit         - Exit the shell\n");
        printf("  jobs         - Show background jobs (not yet implemented)\n");
        printf("  history      - Show command history\n");
        printf("  !n           - Re-run nth command from history\n");
        return 1;
    }

    // jobs command (placeholder)
    else if (strcmp(args[0], "jobs") == 0) {
        printf("Job control not yet implemented.\n");
        return 1;
    }

    // history command
    else if (strcmp(args[0], "history") == 0) {
        for (int i = 0; i < hist_count; i++) {
            printf("%d  %s\n", i + 1, history[i]);
        }
        return 1;
    }

    // Not a built-in command
    return 0;
}

