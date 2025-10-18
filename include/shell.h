#ifndef SHELL_H
#define SHELL_H

#include <stdio.h>   // ✅ Needed for FILE, printf, stdin, etc.
#include <stdlib.h>  // ✅ For malloc, free, exit
#include <string.h>  // ✅ For strcmp, strcpy, strlen
#include <unistd.h>  // ✅ For chdir, fork, execvp

#define MAX_LEN 512
#define MAXARGS 64
#define ARGLEN  64

// Function Prototypes
char* read_cmd(char* prompt, FILE* fp);
char** tokenize(char* cmdline);
int execute(char** arglist);
int handle_builtin(char **args);

#endif

