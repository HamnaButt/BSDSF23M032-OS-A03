#include "shell.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <readline/readline.h>   // ✅ Readline input
#include <readline/history.h>    // ✅ History & completion

#define HISTORY_SIZE 20
char *history_store[HISTORY_SIZE];
int hist_count = 0;

/* ==========================================================
 * read_cmd() — Now uses GNU Readline instead of manual input
 * ========================================================== */
char* read_cmd(char* prompt, FILE* fp) {
    (void)fp;  // fp unused; readline handles its own input

    char *cmdline = readline(prompt);

    if (cmdline == NULL)  // Ctrl+D pressed
        return NULL;

    // If user just pressed Enter, return empty string
    if (strlen(cmdline) == 0) {
        free(cmdline);
        return strdup("");
    }

    // Save non-empty command in Readline’s history
    add_history(cmdline);

    return cmdline;
}

/* ==========================================================
 * tokenize()
 * ========================================================== */
char** tokenize(char* cmdline) {
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
        while (*cp == ' ' || *cp == '\t') cp++;
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

    if (argnum == 0) {
        for(int i = 0; i < MAXARGS + 1; i++) free(arglist[i]);
        free(arglist);
        return NULL;
    }

    arglist[argnum] = NULL;
    return arglist;
}

/* ==========================================================
 * handle_builtin() — same as before
 * ========================================================== */
int handle_builtin(char **args)
{
    if (args == NULL || args[0] == NULL)
        return 1;

    if (strcmp(args[0], "exit") == 0) {
        printf("Exiting shell...\n");
        exit(0);
    }
    else if (strcmp(args[0], "cd") == 0) {
        if (args[1] == NULL)
            fprintf(stderr, "cd: expected argument\n");
        else if (chdir(args[1]) != 0)
            perror("cd");
        return 1;
    }
    else if (strcmp(args[0], "help") == 0) {
        printf("Built-in commands:\n");
        printf("  cd <dir>     — change directory\n");
        printf("  help         — show help message\n");
        printf("  exit         — exit the shell\n");
        printf("  jobs         — placeholder for job control\n");
        printf("  history      — show recent commands\n");
        return 1;
    }
    else if (strcmp(args[0], "jobs") == 0) {
        printf("Job control not yet implemented.\n");
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

    return 0;
}

