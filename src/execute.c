#include "shell.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/wait.h>

/* Expand variables in a token */
static char *expand_string(const char *orig) {
    if (!orig) return NULL;
    if (strchr(orig, '$') == NULL) return strdup(orig);

    char buf[ARGLEN * 4] = {0};
    size_t bi = 0;
    for (size_t i = 0; i < strlen(orig); ) {
        if (orig[i] == '$') {
            i++;
            char name[ARGLEN] = {0};
            int ni = 0;
            while (i < strlen(orig) && (isalnum(orig[i]) || orig[i]=='_')) {
                if (ni < ARGLEN-1) name[ni++] = orig[i];
                i++;
            }
            name[ni] = '\0';
            const char *val = get_var(name);
            if (val) strncat(buf, val, sizeof(buf) - strlen(buf) - 1);
        } else {
            if (bi < sizeof(buf)-1) buf[bi++] = orig[i];
            i++;
        }
    }
    buf[bi] = '\0';
    return strdup(buf);
}

static void expand_args_inplace(char **args) {
    for (int i = 0; args[i]; i++) {
        char *expanded = expand_string(args[i]);
        free(args[i]);
        args[i] = strdup(expanded);
        free(expanded);
    }
}

/* Built-in commands */
static int handle_builtin_execute(char **args) {
    if (!args || !args[0]) return 0;
    if (strcmp(args[0], "exit") == 0) { free_vars(); exit(0); }
    if (strcmp(args[0], "set") == 0) { print_all_vars(); return 1; }
    return 0;
}

/* ================= execute() ================= */
int execute(char **arglist, int background) {
    if (!arglist || !arglist[0]) return 0;

    // Variable assignment
    char *eq = strchr(arglist[0], '=');
    if (eq && eq != arglist[0]) {
        *eq = '\0';
        char *name = arglist[0];
        char *value = eq + 1;
        if (value[0]=='"' && value[strlen(value)-1]=='"') { value[strlen(value)-1]='\0'; value++; }
        set_var(name, value);
        return 0;
    }

    // Expand variables
    expand_args_inplace(arglist);

    // Built-in commands
    if (handle_builtin_execute(arglist)) return 0;

    // Fork + exec
    pid_t pid = fork();
    if (pid==0) {
        execvp(arglist[0], arglist);
        perror("execvp");
        exit(1);
    } else if (pid > 0) {
        if (!background) {
            int status;
            waitpid(pid, &status, 0);
            return status;
        } else {
            add_job(pid, arglist[0]);
            return pid;
        }
    } else {
        perror("fork");
        return -1;
    }
}
int execute_get_status(char **args) {
    if (!args || !args[0]) return -1;

    // Expand variables
    expand_args_inplace(args);

    // Built-in commands return 0 for success
    if (handle_builtin_execute(args)) return 0;

    pid_t pid = fork();
    if (pid == 0) {
        execvp(args[0], args);
        perror("execvp");
        exit(1);
    } else if (pid > 0) {
        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status)) return WEXITSTATUS(status);
        else return -1;
    } else {
        perror("fork");
        return -1;
    }
}
