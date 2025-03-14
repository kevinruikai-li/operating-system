/* main.c */
#include "queue.h"
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

#define ITEMS_PER_THREAD 5

// Producer thread function
void *producer_thread(void *arg) {
    struct queue *q = (struct queue *) arg;
    for (int i = 0; i < ITEMS_PER_THREAD; ++i) {
        int *val = (int *) malloc(sizeof(int));
        if (!val) {
            fprintf(stderr, "Memory allocation error\n");
            exit(EXIT_FAILURE);
        }
        *val = i;
        printf("[Producer] Enqueuing: %d\n", *val);
        enqueue(q, val);
        sleep(1);               // slight delay
    }
    return NULL;
}

// Consumer thread function
void *consumer_thread(void *arg) {
    struct queue *q = (struct queue *) arg;
    for (int i = 0; i < ITEMS_PER_THREAD; ++i) {
        int *val = dequeue(q);
        printf("[Consumer] Dequeued: %d\n", *val);
        free(val);
    }
    return NULL;
}

int main() {
    // Create a synchronized queue
    struct queue *q = make_queue();
    if (!q) {
        fprintf(stderr, "Failed to create queue.\n");
        return EXIT_FAILURE;
    }
    // Create producer and consumer threads
    pthread_t producer, consumer;
    pthread_create(&producer, NULL, producer_thread, q);
    pthread_create(&consumer, NULL, consumer_thread, q);

    // Wait for threads to finish
    pthread_join(producer, NULL);
    pthread_join(consumer, NULL);

    // Destroy the queue
    destroy_queue(q);

    return EXIT_SUCCESS;
}
