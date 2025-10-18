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
        // 1. Display prompt & read command
        cmdline = read_cmd("myshell> ", stdin);

        // Handle Ctrl+D (EOF)
        if (cmdline == NULL) {
            printf("\nExiting shell...\n");
            break;
        }

        // 2. Handle !n command before adding to history
        if (cmdline[0] == '!' && strlen(cmdline) > 1) {
            int n = atoi(cmdline + 1);
            if (n > 0 && n <= hist_count) {
                free(cmdline);
                cmdline = strdup(history[n - 1]);
                printf("%s\n", cmdline);
            } else {
                printf("No such command in history.\n");
                free(cmdline);
                continue;
            }
        }

        // 3. Add to history
        add_to_history(cmdline);

        // 4. Tokenize the command line
        arglist = tokenize(cmdline);

        // 5. Skip empty lines
        if (arglist == NULL) {
            free(cmdline);
            continue;
        }

        // 6. Handle built-ins first
        if (!handle_builtin(arglist)) {
            // 7. Execute external commands
            result = execute(arglist);
        }

        // 8. Free memory
        free(cmdline);
        for (int i = 0; arglist[i] != NULL; i++)
            free(arglist[i]);
        free(arglist);
    }

    return 0;
}

