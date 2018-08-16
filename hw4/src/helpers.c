#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>

#include "helpers.h"
#include "sfish.h"
#include "sf_builtins.h"

char **get_tokens_from_rl(char *line, char *separator, char *pipe) {
    int separators = 0 , i = 0;

    if (line == NULL) return NULL;

    char **tokens = NULL;
    char *p = line;

    // find num of separators
    while (*p) {
        printf("%c\n", *p);
        if (*p == '|' || *p == '<' || *p == '>')
            printf("%c %p\n", *p, p);
        else if (*p == ' ')
            separators++;
        p++;
    }
    // NULL terminated
    tokens = malloc(sizeof(char*) * (separators + 2));

    // build array
    if ((tokens[0] = strtok(line, separator)) != NULL) {
        while ((tokens[++i] = strtok(NULL, separator)) != NULL);
    }

    // free(tokens);
    return tokens;
}

char **get_args_to_delim(char *input, char *delim) {
    if (*input == 0) return NULL;

    char **args = calloc( strlen(input), sizeof(char*) );
    char **current = args;

    *current = strtok(input, delim);
    current++;
    while ( (*current = strtok(NULL, delim)) != NULL ) {
        current++;
    }
    // free
    return args;
}

int validate_input(char *input, int stops[]) {
    char *p = input;
    int accept = 0; // accept = 0, reject = -1
    int i = 0, j = 0;

    // supported alphabet: [a-z], [A-Z], -, [0-9]
    // supported special chars: |, <, >
    if ( in_special(*p) == 0) return -1;
    while (*p) {
        if (in_alphabet(*p) == 0 || *p == ' ') {
            p++;
            i++;
            accept = 0;
        }
        else if (in_special(*p) == 0) {
            stops[j] = i;
            p++;
            i++;
            j++;
            accept = -1;
            while (*p) {
                if (in_special(*p) == 0)
                    return -1;
                else if (*p == ' ') {
                    p++;
                    i++;
                    continue;
                }
                else if (in_alphabet(*p) == 0) {
                    p++;
                    i++;
                    accept = 0;
                    break;
                }
                else
                    return -1;
            }
        }
        else
            return -1;
    }
    return accept;
}

char *home_aware_prompt(char *buf, char *appendTo) {

    char path[1024];
    char *cwd =  _sfb_pwd();
    char *home = getenv("HOME");

    strcpy(path, cwd);

    strncpy(buf, path, strlen(home));
    buf[strlen(home)] = '\0';

    // find if HOME is substr
    if (strcmp(buf, home) == 0) {
        // replace home with ~
        strcpy(buf, "~");
        strcat(buf, (path + strlen(home)) );
    }

    strcat(buf, appendTo);

    return buf;
}

int in_alphabet(char c) {
    return ( c == ALPHABET_DQUOTE ||
             c == ALPHABET_AMP ||
             c == ALPHABET_SQUOTE ||
             c == ALPHABET_L_PAREN ||
             c == ALPHABET_R_PAREN ||
             c == ALPHABET_DASH ||
             c == ALPHABET_DOT ||
             c == ALPHABET_SLASH ||
             c == ALPHABET_COLON ||
             c == ALPHABET_L_BRACKET ||
             c == ALPHABET_BSLASH ||
             c == ALPHABET_R_BRACKET ||
             c == ALPHABET_BACKTICK ||
             c == ALPHABET_UNDERSCORE ||
             c == ALPHABET_TILDE ||
             (c >= DECIMAL_L_MIN && c <= DECIMAL_L_MAX) ||
             (c >= ALPHABET_L_MIN && c <= ALPHABET_L_MAX) ||
             (c >= ALPHABET_U_MIN && c <= ALPHABET_U_MAX )
           ) ? 0 : -1;
}

int in_special(char c) {
    return ( c == '<' || c == '|' || c == '>') ? 0 : -1;
}

