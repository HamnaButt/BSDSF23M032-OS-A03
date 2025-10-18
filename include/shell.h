#ifndef SHELL_H
#define SHELL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>

#define MAX_LEN 512
#define MAXARGS 64
#define ARGLEN  64

// Core shell functions
char* read_cmd(char* prompt, FILE* fp);
char** tokenize(char* cmdline);
// execute returns 0 for foreground completion, or pid (>0) for background start
int execute(char** arglist, int background);
int handle_builtin(char **args);

// job helpers used by main
void reap_jobs(void);
void add_job(pid_t pid, const char *cmd);
void remove_job(pid_t pid);

#endif

