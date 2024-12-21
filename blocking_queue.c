#include "blocking_queue.h"
#include "utilities.h"

// Student : Salameh Alfasatleh ID: 20578169
#include "list.h"             
#include <pthread.h>          
#include <assert.h>           
#include <stdio.h>            

void blocking_queue_terminate(BlockingQueueT* queue) {
  pthread_mutex_lock(&queue->mutex);
  queue->terminated = 1;
  pthread_cond_broadcast(&queue->cond); 
  pthread_mutex_unlock(&queue->mutex);
}

void blocking_queue_create(BlockingQueueT* queue) {
  queue->list = list_create();
  pthread_mutex_init(&queue->mutex, NULL);
  pthread_cond_init(&queue->cond, NULL);
  queue->terminated = 0;
  queue->length = 0;
}

void blocking_queue_destroy(BlockingQueueT* queue) {
  pthread_mutex_destroy(&queue->mutex);
  pthread_cond_destroy(&queue->cond);
  list_destroy(queue->list);
}

void blocking_queue_push(BlockingQueueT* queue, unsigned int value) {
  pthread_mutex_lock(&queue->mutex);
  list_append(queue->list, value);
  queue->length++;
  pthread_cond_signal(&queue->cond); 
  pthread_mutex_unlock(&queue->mutex);
}

int blocking_queue_pop(BlockingQueueT* queue, unsigned int* value) {
  pthread_mutex_lock(&queue->mutex);

  while (queue->length == 0 && !queue->terminated) {
    pthread_cond_wait(&queue->cond, &queue->mutex);
  }

  if (queue->terminated) {
    pthread_mutex_unlock(&queue->mutex);
    return -1;  
  }

  *value = list_pop_front(queue->list);
  queue->length--;

  pthread_mutex_unlock(&queue->mutex);
  return 0; 
}

int blocking_queue_empty(BlockingQueueT* queue) {
  pthread_mutex_lock(&queue->mutex);
  int is_empty = (queue->length == 0);
  pthread_mutex_unlock(&queue->mutex);
  return is_empty;
}

int blocking_queue_length(BlockingQueueT* queue) {
  pthread_mutex_lock(&queue->mutex);
  int length = queue->length;
  pthread_mutex_unlock(&queue->mutex);
  return length;;
}
