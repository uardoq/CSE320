#include "handlers.h"
#include <sys/wait.h>
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

// temporary handler for SIGCHLD.
void sigchld_handler(pid_t pid) {
    int status;
    pid_t cpid = wait(&status);
    if (cpid) {}
    printf("handler %d", STDIN_FILENO);
    debug("SIGCHLD received, reaping child: %d : status: %d\n", (int) cpid, status);
}

int exec_proc(char **input_args, int pipefdin, int pipefdout) {

    struct sigaction action, oldaction;
    action.sa_handler = sigchld_handler;
    sigemptyset(&action.sa_mask);
    action.sa_flags = SA_RESTART;

    if (sigaction(SIGCHLD, &action, &oldaction) < 0) {
        printf(EXEC_ERROR, strerror(errno));
        return -1;
    }

    // create proc mask
    sigset_t sigmask, oldsigmask;
    // inits mask
    sigfillset(&sigmask);
    // set SIGCHLD
    sigdelset(&sigmask, SIGCHLD);
    if (sigprocmask(SIG_SETMASK, &sigmask, &oldsigmask) == -1) {
        printf(EXEC_ERROR, strerror(errno));
        free(input_args);
        return -1;
    }

    // fork and exec
    pid_t pid = fork();
    if (pid < 0) {
        printf(EXEC_ERROR, strerror(errno));
        free(input_args);
    }
    if (pid == 0) {
        printf("child %d", STDIN_FILENO);
        // child
        debug("child pid: %d working...", getpid());
        if (execvp(input_args[0], input_args) == -1) {
            if (errno == ENOENT) printf(EXEC_NOT_FOUND, input_args[0]);
            else printf(EXEC_ERROR, strerror(errno));
            free(input_args);
        }
    }
    printf("parent %d", STDIN_FILENO);

    // parent wait until child dies
    sigsuspend(&sigmask);
    // restore old mask
    sigprocmask(SIG_SETMASK, &oldsigmask, NULL);
    debug("%s", "child is done. Parent ack.");

    return 0;
}
