#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <readline/readline.h>
#include <errno.h>
#include <sys/wait.h>

#include "sfish.h"
#include "debug.h"
#include "sf_builtins.h"
#include "helpers.h"
#include "handlers.h"

int main(int argc, char *argv[], char* envp[]) {
    char* input;
    bool exited = false;
    char prompt[1024];

    if (!isatty(STDIN_FILENO)) {
        // If your shell is reading from a piped file
        // Don't have readline write anything to that file.
        // Such as the prompt or "user input"
        if ((rl_outstream = fopen("/dev/null", "w")) == NULL) {
            perror("Failed trying to open DEVNULL");
            exit(EXIT_FAILURE);
        }
    }
    do {
        input = readline(home_aware_prompt(prompt, " :: equispe >> "));

        if (input == NULL) {
            rl_free(input);
            continue;
        }
        // run if builtin
        int _exited = 0;
        if (run_if_builtin(input, &_exited) >= 0) {
            if (_exited == 1) exited = true;
            continue;
        }

        // not a builtin
        char *subinput = NULL;
        int stops[strlen(input)];

        // init stops array
        for (int i = 0; i < strlen(input); ++i) stops[i] = -1;

        // validate input for executable, build stops array
        if (validate_input(input, stops) == -1 ) {
            printf(SYNTAX_ERROR, "invalid syntax: |,<,> followed by |,<,>, or |,<,> at begining or end");
            rl_free(input); //
            return EXIT_FAILURE;
        }

        // if no stops, treat input as single command
        if (stops[0] == -1 ) {
            char *args[strlen(input)];
            int k = 0;
            if (( *(args + k) = strtok(input, " ")) != NULL ) {
                k++;
                while ((*(args + k++) = strtok(NULL, " ")) != NULL);
            }
            k++;
            *(args + k) = NULL;
            exec_proc(args, 0, 0);      //
            rl_free(input);
            continue;
        }

        // process each piece of the input string separated by |, <, >
        // assumes that there is at least one string
        int i = 0;
        int iopipe[2];
        do {
            // get chars up to a stop
            // add 1 for NULL terminating \0, needed for execve
            subinput = malloc(sizeof(char*) * stops[i] + 1);   //
            memcpy(subinput, input, stops[i]);
            // add null terminator
            *(subinput + stops[i]) = 0;

            // split string by tokens
            // check if var input was changed by strtok
            char **subtokens = get_args_to_delim(subinput, " ");

            if (pipe(iopipe) == -1) {
                free(subinput);
                rl_free(input);
                break;
            }

            dup2(STDIN_FILENO, iopipe[1]);

            // run executable
            exec_proc(subtokens, iopipe[0], iopipe[1]);

            // if executable followed by pipe, pipe stdout to next process

            free(subinput);
            rl_free(input);
            i++;
        } while ( *(stops + i) != -1 );

    } while (!exited);

    sfb_exit();

    return EXIT_SUCCESS;
}
