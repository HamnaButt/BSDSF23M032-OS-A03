#include "shell.h"
#include <stdio.h>
#include <stdlib.h>

int main()
{
    char *cmdline;
    char **arglist;
    int result;

    while (1) {
        // 1. Display prompt & read command
        cmdline = read_cmd("myshell> ", stdin);

        // Handle Ctrl+D (EOF)
        if (cmdline == NULL) {
            printf("\nExiting shell...\n");
            break;
        }

        // 2. Tokenize the command line
        arglist = tokenize(cmdline);

        // 3. Skip empty lines
        if (arglist == NULL) {
            free(cmdline);
            continue;
        }

        // 4. Handle built-ins first
        if (!handle_builtin(arglist)) {
            // 5. Execute external commands
            result = execute(arglist);
        }

        // 6. Free memory
        free(cmdline);
        for (int i = 0; arglist[i] != NULL; i++)
            free(arglist[i]);
        free(arglist);
    }

    return 0;
}

