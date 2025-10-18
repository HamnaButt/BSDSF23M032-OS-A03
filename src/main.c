#include "shell.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main()
{
    char *cmdline;
    char **arglist;
    int result;

    while (1) {
        // Readline handles prompt, editing, and history
        cmdline = read_cmd("myshell> ", stdin);

        if (cmdline == NULL) {  // Ctrl+D exits
            printf("\nExiting shell...\n");
            break;
        }

        // Skip empty input
        if (strlen(cmdline) == 0) {
            free(cmdline);
            continue;
        }

        // Split into tokens
        arglist = tokenize(cmdline);
        if (arglist == NULL) {
            free(cmdline);
            continue;
        }

        // Handle built-in or external command
        if (!handle_builtin(arglist))
            result = execute(arglist);

        // Free memory
        free(cmdline);
        for (int i = 0; arglist[i] != NULL; i++)
            free(arglist[i]);
        free(arglist);
    }

    return 0;
}

