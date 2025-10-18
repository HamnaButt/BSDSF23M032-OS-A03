#include "shell.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

/* -------------------------------------------------
 * Helper: find the position of the '|' operator
 * ------------------------------------------------- */
int find_pipe(char **args) {
    for (int i = 0; args[i] != NULL; i++) {
        if (strcmp(args[i], "|") == 0)
            return i;
    }
    return -1;
}

/* -------------------------------------------------
 * execute(): handles redirection (<, >) and piping (|)
 * ------------------------------------------------- */
int execute(char **args)
{
    int pipe_pos = find_pipe(args);

    /* -----------------------------
     * Case 1 — Piped Commands
     * ----------------------------- */
    if (pipe_pos != -1) {
        args[pipe_pos] = NULL;
        char **left = args;
        char **right = &args[pipe_pos + 1];

        int fd[2];
        if (pipe(fd) == -1) {
            perror("pipe");
            return 1;
        }

        pid_t pid1 = fork();
        if (pid1 == 0) {
            dup2(fd[1], STDOUT_FILENO);  // stdout → pipe write
            close(fd[0]);
            close(fd[1]);
            execvp(left[0], left);
            perror("execvp (left)");
            exit(1);
        }

        pid_t pid2 = fork();
        if (pid2 == 0) {
            dup2(fd[0], STDIN_FILENO);   // stdin ← pipe read
            close(fd[0]);
            close(fd[1]);
            execvp(right[0], right);
            perror("execvp (right)");
            exit(1);
        }

        close(fd[0]);
        close(fd[1]);
        waitpid(pid1, NULL, 0);
        waitpid(pid2, NULL, 0);
        return 0;
    }

    /* -----------------------------
     * Case 2 — Redirection (<, >)
     * ----------------------------- */
    int in_redirect = -1, out_redirect = -1;
    for (int i = 0; args[i] != NULL; i++) {
        if (strcmp(args[i], "<") == 0)
            in_redirect = i;
        else if (strcmp(args[i], ">") == 0)
            out_redirect = i;
    }

    pid_t pid = fork();
    if (pid == 0) {
        // Input <
        if (in_redirect != -1) {
            int fd = open(args[in_redirect + 1], O_RDONLY);
            if (fd < 0) {
                perror("open input");
                exit(1);
            }
            dup2(fd, STDIN_FILENO);
            close(fd);
            args[in_redirect] = NULL;
        }

        // Output >
        if (out_redirect != -1) {
            int fd = open(args[out_redirect + 1],
                          O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd < 0) {
                perror("open output");
                exit(1);
            }
            dup2(fd, STDOUT_FILENO);
            close(fd);
            args[out_redirect] = NULL;
        }

        execvp(args[0], args);
        perror("execvp");
        exit(1);
    }
    else if (pid > 0) {
        waitpid(pid, NULL, 0);
    }
    else {
        perror("fork");
    }

    return 0;
}

