#include <pthread.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>

#include "cream.h"
#include "queue.h"
#include "hashmap.h"
#include "utils.h"
#include "csapp.h"
#include "debug.h"
#include "wrappers.h"

queue_t *queue;
hashmap_t *hashmap;

void *thread (void *argp) {
    debug("detaching thread: %lu\n", (long)pthread_self());
    Pthread_detach(pthread_self());
    int *connfd = NULL;
    while (true) {
        debug("thread %lu is entering dequeue", (long)pthread_self());
        if ((connfd = dequeue(queue)) == NULL)
            debug("%s", "could not dequeue connfd");
        debug("connfd: %d", *connfd);

        service_request(*connfd, &hashmap);

        Close(*connfd);
        debug("connfd closed: %d", *connfd);
        free(connfd);
    }
}

void map_free_function(map_key_t key, map_val_t val) {
    free(key.key_base);
    free(val.val_base);
}

void sigint_handler(int signo) {
    // if the server receives an SIGINT signal
    // wait until all requests are finished before exiting the server.

    signal(SIGINT, SIG_IGN);

    debug("int got: %d\n", signo);

    printf("%s\n", "shutting down....");

    sleep(2);

    pthread_mutex_destroy(&queue->lock);
    pthread_mutex_destroy(&hashmap->write_lock);
    pthread_mutex_destroy(&hashmap->fields_lock);

    exit(EXIT_SUCCESS);
}

void epipe_handler(int signo) {
    debug("epipe got: %d\n", signo);

}

void sigpipe_handler(int signo) {
    debug("sigpipe jgot: %d\n", signo);

}


int main(int argc, char *argv[]) {
    int NUM_WORKERS;
    char *PORT_NUMBER;
    int MAX_ENTRIES;

    int listenfd, *connfd;
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;
    pthread_t tid;

    handle_args(argc, argv, &NUM_WORKERS, &PORT_NUMBER, &MAX_ENTRIES);

    if ((queue = create_queue()) == NULL) {
        debug("%s\n", "error creating queue");
        exit(EXIT_FAILURE);
    }
    if ((hashmap = create_map(MAX_ENTRIES, jenkins_one_at_a_time_hash, map_free_function)) == NULL) {
        debug("%s\n", "error creating hashmap");
        exit(EXIT_FAILURE);
    }

    if ( (listenfd = Open_listenfd(PORT_NUMBER)) == -1) {
        debug("%s\n", "error creating listenfd");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < NUM_WORKERS; ++i) {
        Pthread_create(&tid, NULL, thread, NULL);
    }

    while (true) {
        clientlen = sizeof(struct sockaddr_storage);
        debug("%s\n", "waiting for connection");
        if ((connfd = Calloc(1, sizeof(int))) == NULL ) {
            exit(EXIT_FAILURE);
        }
        if ( (*connfd = Accept(listenfd, (SA*)&clientaddr, &clientlen)) == -1) {
            debug("%s\n", "error creating listenfd");
            exit(EXIT_FAILURE);
        }
        debug("%s\n", "accepted connection");

        if (enqueue(queue, connfd) == false) {
            debug("error queueing connfd: %d", *connfd);
        }
    }

    exit(EXIT_SUCCESS);
}
