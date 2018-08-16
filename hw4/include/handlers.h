#ifndef HANDLERS_H
#define HANDLERS_H

void sigchld_handler();

int exec_proc(char **input_args, int pipefdin, int pipefdout);

#endif