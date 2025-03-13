/* queue.c */
#include "queue.h"
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>


struct queue *
make_queue ()
{
  // Allocate memory and initialize synchronization mechanisms
  struct queue *q = malloc (sizeof (struct queue));
  if (!q)
    {
      return NULL;
    }
  q->head = 0;
  q->tail = 0;
  q->size = 0;
  sem_init (&q->spaces, 0, 0);
  return q;
}

void
enqueue (struct queue *q, void *item)
{
  // Add item to the queue and update necessary pointers
  pthread_mutex_lock (&q->mutex);
  struct queue_node *node = malloc (sizeof (struct queue_node));
  if (!node)
    {
      fprintf (stderr, "Memory allocation error\n");
      exit (EXIT_FAILURE);
    }
  node->item = item;
  node->next = 0;
  node->prev = q->tail;
  if (q->tail)
    {
      q->tail->next = node;
    }
  q->tail = node;
  if (!q->head)
    {
      q->head = node;
    }
  q->size++;
  pthread_mutex_unlock (&q->mutex);
  sem_post (&q->spaces);
}

void *
dequeue (struct queue *q)
{
  // Retrieve an item while ensuring thread safety
  sem_wait (&q->spaces);
  pthread_mutex_lock (&q->mutex);
  struct queue_node *node = q->head;
  q->head = node->next;
  if (q->head)
    {
      q->head->prev = 0;
    }
  if (node == q->tail)
    {
      q->tail = 0;
    }
  q->size--;
  pthread_mutex_unlock (&q->mutex);
  void *item = node->item;
  free (node);
  return item;
}

void
destroy_queue (struct queue *q)
{
  // Clean up allocated resources and synchronization mechanisms
  while (q->head)
    {
      struct queue_node *node = q->head;
      q->head = node->next;
      free (node);
    }
  sem_destroy (&q->spaces);
  free (q);
}
