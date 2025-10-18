#ifndef SHELL_H
#define SHELL_H

#include <stdio.h>   // for FILE, printf
#include <stdlib.h>  // for malloc, free, exit
#include <string.h>  // for strcmp, strcpy
#include <unistd.h>  // for chdir, fork, execvp

#define MAX_LEN 512
#define MAXARGS 64
#define ARGLEN  64

// Function Prototypes
char* read_cmd(char* prompt, FILE* fp);
char** tokenize(char* cmdline);
int execute(char** arglist);
int handle_builtin(char **args);
void add_to_history(char *cmdline);

// History globals (declared in shell.c)
extern char *history[];
extern int hist_count;

#endif

