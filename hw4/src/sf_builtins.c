#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdbool.h>

#include "sf_builtins.h"
#include "sfish.h"

char *oldpwd;

int run_if_builtin(char *input, int *exited) {

    // make copy of original string because strtok destroys string
    char *cpy = calloc(strlen(input), sizeof(char));
    char *tokens[2];

    memcpy(cpy, input, strlen(input));

    tokens[0] = strtok(cpy, " ");
    tokens[1] = strtok(NULL, " ");

    if (tokens[0] == NULL) {
        free(cpy);
        return 1;
    }
    else if (strcmp(tokens[0], "cd") == 0) {
        // debug("%s", "user entered 'cd'");
        sfb_cd(tokens[1]);
        free(cpy);
        return 0;
    }
    else if (strcmp(tokens[0], "exit") == 0) {
        // debug("%s", "user entered 'exit'");
        *exited = 1;
        free(cpy);
        return 0;
    }
    else if (strcmp(tokens[0], "help") == 0) {
        // debug("%s", "user entered 'help'");
        sfb_help();
        free(cpy);
        return 0;
    }
    else if (strcmp(tokens[0], "pwd") == 0) {
        // debug("%s", "user entered 'pwd'");
        sfb_pwd();
        free(cpy);
        return 0;
    }
    else {
        // builtin not run
        free(cpy);
        return -1;
    }
}

void sfb_help() {
    printf("%s", PRINT_HELP);
}

void sfb_exit() {
    free(oldpwd);
    _exit(0);
}

int sfb_cd(char* arg) {
    char *tmp;

    // SEG FAULTS
    if (arg == NULL) {
        // save oldpwd
        tmp = (char*)calloc(1024, sizeof(char));
        getcwd(tmp, 1024);
        // go home
        if (chdir(getenv("HOME")) == -1) {
            printf(BUILTIN_ERROR, strerror(errno));
            return -1;
        }
        if (oldpwd != NULL) {
            free(oldpwd);
        }
        oldpwd = tmp;

    }
    else if (strcmp(arg, "-") == 0) {
        if (oldpwd == NULL) {
            printf(BUILTIN_ERROR, "NO OLDPWD set");
            return -1;
        }
        tmp = (char*)calloc(1024, sizeof(char));
        // load wd before overwritting wd
        getcwd(tmp, 1024);
        // overwrite wd with old wd
        chdir(oldpwd);
        // save wd as old wd
        free(oldpwd);
        oldpwd = tmp;
    }
    else {
        // save oldpwd
        tmp = (char*)calloc(1024, sizeof(char));
        getcwd(tmp, 1024);
        // . || .. || path
        if (chdir(arg) == -1) {
            printf(BUILTIN_ERROR, strerror(errno));
            return -1;
        }
        free(oldpwd);
        oldpwd = tmp;
    }
    return 0;
}

void sfb_pwd() {
    char *buf;
    buf = _sfb_pwd();
    if (buf != NULL) {
        printf("%s\n", buf);
        free(buf);
    }
}

/* must free outside */
char *_sfb_pwd() {
    int req_size = 64, mult = 1;
    char *buf, *rslt;

    buf = (char*)calloc(req_size, sizeof(char));
    if (errno == ENOMEM) {
        printf(BUILTIN_ERROR, "ENOMEM when allocating vm for path");
        return NULL;
    }
    rslt = getcwd(buf, req_size * mult);
    while (rslt == NULL && errno == ERANGE) {
        buf = (char*)realloc(buf, ++mult * req_size);
        if (errno == ENOMEM) {
            printf(BUILTIN_ERROR, "ENOMEM when allocating vm for path");
            return NULL;
        }
        rslt = getcwd(buf, req_size * mult);
    }

    return buf;
}