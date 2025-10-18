#include "shell.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

/* Helper: find pipe token index; returns -1 if none */
int find_pipe(char **arglist) {
    for (int i = 0; arglist[i] != NULL; i++) {
        if (strcmp(arglist[i], "|") == 0) return i;
    }
    return -1;
}

/* execute: if background == 1 then parent does NOT wait and function returns pid (>0)
   if background == 0 then blocks until child(s) complete and returns 0 */
int execute(char **args, int background)
{
    int pipe_index = find_pipe(args);

    /* ---------- Piped commands ---------- */
    if (pipe_index != -1) {
        args[pipe_index] = NULL;
        char **left = args;
        char **right = &args[pipe_index + 1];

        int fd[2];
        if (pipe(fd) == -1) {
            perror("pipe");
            return -1;
        }

        pid_t pid1 = fork();
        if (pid1 == 0) {
            dup2(fd[1], STDOUT_FILENO);
            close(fd[0]);
            close(fd[1]);
            execvp(left[0], left);
            perror("execvp (left)");
            exit(1);
        }

        pid_t pid2 = fork();
        if (pid2 == 0) {
            dup2(fd[0], STDIN_FILENO);
            close(fd[0]);
            close(fd[1]);
            execvp(right[0], right);
            perror("execvp (right)");
            exit(1);
        }

        // parent closes pipe ends
        close(fd[0]);
        close(fd[1]);

        if (background) {
            // In background: do not wait; return pid2 as job pid
            return pid2 > 0 ? pid2 : pid1;
        } else {
            waitpid(pid1, NULL, 0);
            waitpid(pid2, NULL, 0);
            return 0;
        }
    }

    /* ---------- Redirection / normal execution ---------- */
    int in_redirect = -1, out_redirect = -1, append_redirect = -1;
    for (int i = 0; args[i] != NULL; i++) {
        if (strcmp(args[i], "<") == 0) in_redirect = i;
        else if (strcmp(args[i], ">") == 0) out_redirect = i;
        else if (strcmp(args[i], ">>") == 0) append_redirect = i;
    }

    pid_t pid = fork();
    if (pid == 0) {
        // Input redirect
        if (in_redirect != -1) {
            int fd = open(args[in_redirect + 1], O_RDONLY);
            if (fd < 0) { perror("open input"); exit(1); }
            dup2(fd, STDIN_FILENO);
            close(fd);
            args[in_redirect] = NULL;
        }
        // Output overwrite
        if (out_redirect != -1) {
            int fd = open(args[out_redirect + 1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd < 0) { perror("open output"); exit(1); }
            dup2(fd, STDOUT_FILENO);
            close(fd);
            args[out_redirect] = NULL;
        }
        // Append
        if (append_redirect != -1) {
            int fd = open(args[append_redirect + 1], O_WRONLY | O_CREAT | O_APPEND, 0644);
            if (fd < 0) { perror("open append"); exit(1); }
            dup2(fd, STDOUT_FILENO);
            close(fd);
            args[append_redirect] = NULL;
        }

        execvp(args[0], args);
        perror("execvp");
        exit(1);
    }
    else if (pid > 0) {
        if (background) {
            // Do not wait; parent returns pid for job tracking
            return pid;
        } else {
            int status;
            waitpid(pid, &status, 0);
            return 0;
        }
    }
    else {
        perror("fork");
        return -1;
    }
}

