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

// New: helper that runs a raw command line (does tokenization, expansion, builtins)
// returns pid (>0) if started in background, or exit status (0..255) for foreground success/failure,
// or negative on error.
int run_command_line(char *rawline, int background);

// New: used by if-then-else to get the exit status of a tokenized command
int execute_get_status(char **args);

// job helpers used by main
void reap_jobs(void);
void add_job(pid_t pid, const char *cmd);
void remove_job(pid_t pid);

// Variable helpers
void set_var(const char *name, const char *value);
const char *get_var(const char *name);
void print_all_vars(void);
void free_vars(void);

#endif
