#include "queue.h"
#include "stdlib.h"
#include "semaphore.h"
#include "const.h"
#include "errno.h"
#include <stdio.h>
#include "debug.h"
#include "wrappers.h"
#include "csapp.h"

// implements a queue using a singly linked list
queue_t *create_queue(void) {
    // allocate queue on heap
    queue_t *queue = Calloc(1, sizeof(queue_t));

    if (queue == NULL) {
        return NULL;
    }
    queue->front = queue->rear = NULL;
    // init semaphores
    // start with empty semaphores
    if ( sem_init(&queue->items, 0, 0) == -1 ) {
        // error
        return NULL;
    }
    if ( pthread_mutex_init(&queue->lock, NULL ) == -1 ) {
        return NULL;
    }
    queue->invalid = false;

    return queue;
}

bool invalidate_queue(queue_t *self, item_destructor_f destroy_function) {

    pthread_mutex_lock(&self->lock);

    // invalid args
    if ( self == NULL || self->invalid == true ) {
        errno = EINVAL;
        pthread_mutex_unlock(&self->lock);
        return false;
    }

    if (self->front != NULL) {
        queue_node_t *node = self->front;
        do {
            /* call destroy_function on all remaining items in the queue */
            destroy_function(node);
            /* and free(3) the queue_node_t instances */
            free(node);
        } while ( (node = node->next) != NULL );
    }

    // set invalid flag
    self->invalid = true;

    pthread_mutex_unlock(&self->lock);

    return true;
}

bool enqueue(queue_t *self, void *item) {
    debug("%s\n", "entered enqueue");

    // invalid args
    if ( self == NULL || self->invalid == true ) {
        errno = EINVAL;
        return false;
    }

    pthread_mutex_lock(&self->lock);
    debug("%s\n", "locked in enqueue");

    // allocate new node instance
    queue_node_t *node = Calloc(1, sizeof(queue_node_t));
    // point item ptr to our new node.
    // when we want to free this item, we can call free(node->item)
    node->item = item;
    node->next = NULL;

    /* critical section */

    // queue node
    if (self->rear == NULL && self->rear == NULL) {
        debug("%s\n", "empty queue case");
        // case empty queue
        self->front = self->rear = node;
        // notify dequeue process that there is an item in the queue
        sem_post(&self->items);
        // release lock to queue

        pthread_mutex_unlock(&self->lock);
        debug("%s\n", "unlocked in enqueue");
        return true;
    }
    else {
        debug("%s\n", "non-empty queue case");
        // Front    Rear
        //     [ A ] -> NULL
        // [ A ] -> [ B ] -> NULL
        // make previous rear point to it and make new node the rear
        self->rear->next = node;
        self->rear = node;
        sem_post(&self->items);
        pthread_mutex_unlock(&self->lock);
        debug("%s\n", "unlocked in enqueue");
        return true;
    }
    /* critical section */

    return false;
}

/* Consumer. P() */
void *dequeue(queue_t *self) {
    debug("%s\n", "entered dequeue");
    // invalid args
    if ( self == NULL || self->invalid == true ) {
        errno = EINVAL;
        return false;
    }

    queue_node_t *dequeuedNode = NULL;
    // if items > 0, then dequeue else wait for producer to put items in list
    sem_wait(&self->items);
    // at this point, since items > 0 acquire queue lock and dequeue
    // if lock not available wait until its available.
    debug("%s\n", "lock in dequeue" );
    pthread_mutex_lock(&self->lock);

    dequeuedNode = self->front;
    int *item = (int*)dequeuedNode->item;
    // if only 1 node in list
    // Front    Rear
    //     [ A ] -> NULL
    if (self->front->next == NULL) {
        debug("only 1 node in queue %p\n", dequeuedNode);
        self->front = self->rear = NULL;
    }
    else {
        debug("more than 1 node in queue%p\n", dequeuedNode);
        // Front    Rear
        // [ A ] -> [ B ] -> NULL
        // dequeue from the front of the list, so A is removed.

        // make the next node in queue the new front
        self->front = self->front->next;
    }

    free(dequeuedNode);

    // release access to the lock
    debug("%s\n", "unlocked in dequeue");
    pthread_mutex_unlock(&self->lock);

    // return the dequeued node
    return item;
}
