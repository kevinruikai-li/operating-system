/* test_queue.c */
#include "queue.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <assert.h>
#include <unistd.h>

int score = 0;
#define NUM_THREADS 5
#define ITEMS_PER_THREAD 10
struct queue *q = NULL;

void print_result(const char *test_name, int passed) {
    if (passed) {
        printf("[PASS] %s\n", test_name);
        score += 1;
    } else {
        printf("[FAIL] %s\n", test_name);
    }
}

void *producer_thread(void *arg) {
    (void) arg;
    for (int i = 0; i < ITEMS_PER_THREAD; ++i) {
        int *val = (int *) malloc(sizeof(int));
        if (!val) {
            fprintf(stderr, "Memory allocation error\n");
            exit(EXIT_FAILURE);
        }
        *val = i;
        enqueue(q, val);
        sleep(1);
    }
    return NULL;
}

void *consumer_thread(void *arg) {
    (void) arg;
    for (int i = 0; i < ITEMS_PER_THREAD; ++i) {
        int *val = NULL;
        val = (int *) dequeue(q);
        assert(val != NULL);
        free(val);
    }
    return NULL;
}

void test_queue_order() {
    q = make_queue();
    int a = 1, b = 2, c = 3;
    enqueue(q, &a);
    enqueue(q, &b);
    enqueue(q, &c);

    int *res1 = (int *) dequeue(q);
    int *res2 = (int *) dequeue(q);
    int *res3 = (int *) dequeue(q);

    int passed = (res1 == &a && res2 == &b && res3 == &c);
    print_result("Queue Order Test", passed);
    destroy_queue(q);
}

void test_enqueue_null_destroy() {
    q = make_queue();
    // attempt to add only a NULL pointer to the queue,
    // then destroy the queue without removing it.
    enqueue(q, NULL);
    destroy_queue(q);
    print_result("Null Destruction Test", 1);   // The program will segfault if this fails.
}

void test_stress_queue() {
    q = make_queue();
    pthread_t producers[NUM_THREADS], consumers[NUM_THREADS];

    for (int i = 0; i < NUM_THREADS; ++i) {
        pthread_create(&producers[i], NULL, producer_thread, NULL);
        pthread_create(&consumers[i], NULL, consumer_thread, NULL);
    }

    for (int i = 0; i < NUM_THREADS; ++i) {
        pthread_join(producers[i], NULL);
        pthread_join(consumers[i], NULL);
    }
    print_result("Stress Queue Test", 1);
    destroy_queue(q);
}

void test_queue_size() {
    q = make_queue();
    int a = 1, b = 2;
    enqueue(q, &a);
    enqueue(q, &b);
    int *res = (int *) dequeue(q);
    print_result("Queue Size Test", res == &a);
    destroy_queue(q);
}

void test_multiple_threads() {
    q = make_queue();
    pthread_t producers[NUM_THREADS], consumers[NUM_THREADS];

    for (int i = 0; i < NUM_THREADS; ++i) {
        pthread_create(&producers[i], NULL, producer_thread, NULL);
        pthread_create(&consumers[i], NULL, consumer_thread, NULL);
    }

    for (int i = 0; i < NUM_THREADS; ++i) {
        pthread_join(producers[i], NULL);
        pthread_join(consumers[i], NULL);
    }
    print_result("Multiple Threads Test", 1);
    destroy_queue(q);
}

void *test_dequeue_from_empty_producer(void *unused) {
    int *a = malloc(sizeof(int));
    *a = 155;
    sleep(1);
    enqueue(q, a);
    return NULL;
}

void test_dequeue_from_empty() {
    q = make_queue();

    pthread_t producer;
    // start the producer thread
    pthread_create(&producer, NULL, test_dequeue_from_empty_producer, NULL);
    // immediately try and dequeue: producer is asleep, and this should block
    // until it can get the value from the producer.
    int *res = dequeue(q);
    print_result("Dequeue from Empty Queue Test", *res == 155);
    destroy_queue(q);
    free(res);

    // only join with producer thread at this point. We don't want to wait for it
    // before dequeuing.
    pthread_join(producer, NULL);
}

void test_enqueue_dequeue_multiple() {
    q = make_queue();
    int a = 1, b = 2, c = 3, d = 4, e = 5;
    enqueue(q, &a);
    enqueue(q, &b);
    enqueue(q, &c);
    enqueue(q, &d);
    enqueue(q, &e);

    int passed = (dequeue(q) == &a && dequeue(q) == &b &&
                  dequeue(q) == &c && dequeue(q) == &d && dequeue(q) == &e);
    print_result("Enqueue-Dequeue Multiple Items Test", passed);
    destroy_queue(q);
}

void test_queue_destruction() {
    q = make_queue();
    destroy_queue(q);
    print_result("Queue Destruction Test", 1);
}

void test_high_load() {
    q = make_queue();
    for (int i = 0; i < 10000; ++i) {
        int *val = (int *) malloc(sizeof(int));
        if (!val) {
            fprintf(stderr, "Memory allocation error\n");
            exit(EXIT_FAILURE);
        }
        *val = i;
        enqueue(q, val);
    }
    for (int i = 0; i < 10000; ++i) {
        int *val = (int *) dequeue(q);
        free(val);
    }
    print_result("High Load Test", 1);
    destroy_queue(q);
}

void *unpredictable_order_producer(void *v) {
    enqueue(q, v);
    return NULL;
}

void test_unpredictable_order() {
    q = make_queue();
    pthread_t producers[2];
    int values[2];
    for (int i = 0; i < 2; ++i) {
        values[i] = rand();
        pthread_create(&producers[i], NULL, unpredictable_order_producer,
                       &values[i]);
    }
    for (int i = 0; i < 2; ++i) {
        pthread_join(producers[i], NULL);
    }

    int results[2];
    results[0] = *(int *) dequeue(q);
    results[1] = *(int *) dequeue(q);

    print_result("Unpredictable Order Test",
                 (results[0] == values[0] && results[1] == values[1])
                 || (results[0] == values[1] && results[1] == values[0])
        );

}

int main() {
    test_queue_order();
    test_enqueue_null_destroy();
    test_stress_queue();
    test_queue_size();
    test_multiple_threads();
    test_dequeue_from_empty();
    test_enqueue_dequeue_multiple();
    test_queue_destruction();
    test_high_load();
    test_unpredictable_order();

    printf("Final Score: %d/10\n", score);
    return 0;
}
