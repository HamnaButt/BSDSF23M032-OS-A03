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

/* Core shell functions */
char* read_cmd(char* prompt, FILE* fp);
char** tokenize(char* cmdline);
int handle_builtin(char **args);

/* Execution */
int execute(char **arglist, int background);
int run_command_line(char *rawline, int background);
// Add under Execution section
int execute_get_status(char **args);

/* Job helpers */
void reap_jobs(void);
void add_job(pid_t pid, const char *cmd);
void remove_job(pid_t pid);

/* Variable helpers */
void set_var(const char *name, const char *value);
const char *get_var(const char *name);
void print_all_vars(void);
void free_vars(void);

#endif
