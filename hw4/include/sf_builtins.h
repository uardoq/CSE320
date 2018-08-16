#ifndef SF_BUILTINS_H
#define SF_BUILTINS_H

void sfb_help();

void sfb_exit();

char *_sfb_pwd();

void sfb_pwd();

int sfb_cd(char* arg);

// return 0 if builtin ran successfully
// return -1 if error happened
// return  1 if no input
int run_if_builtin(char *input, int *exited);

#endif
