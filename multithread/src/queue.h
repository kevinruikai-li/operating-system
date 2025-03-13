/* queue.h */
#ifndef QUEUE_H
#   define QUEUE_H

#   include <pthread.h>
#   include <stdio.h>
#   include <stdlib.h>
#   include <errno.h>
#   include <unistd.h>

#   include <semaphore.h>

// A node in our doubly-linked queue
// You can modify this if you want, but you shouldn't need to.
typedef struct queue_node {
    void *item;
    struct queue_node *next;
    struct queue_node *prev;
} queue_node_t;

// Overall queue structure.
struct queue {
    // fill in whatever additional fields you want for your queue structure.
    // If this doesn't include at least a lock and a semaphore, something is probably wrong!
};

// Function prototypes:

// Allocate a new, empty, properly initialized synchronized queue.
struct queue *make_queue();
// Enqueue the given item to the queue.
void enqueue(struct queue *q, void *item);
// Dequeue the given item from the queue. This should be a blocking operation:
// if the queue is empty, the calling thread is blocked until an item is available.
void *dequeue(struct queue *q);
// Destroy the given queue, freeing or releasing any resources that it was using.
// This does NOT include freeing the data items -- that is the user's responsibility.
// It does include freeing all of the nodes, and any bookkeeping or synchronization resources
// that belonged to the queue.
void destroy_queue(struct queue *q);

#endif // QUEUE_H
