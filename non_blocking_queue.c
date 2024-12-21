#include "non_blocking_queue.h"
#include "utilities.h"

#include <assert.h>
// Student : Salameh Alfasatleh ID: 20578169
#include <pthread.h>
#include "list.h"


void non_blocking_queue_create(NonBlockingQueueT* queue) {
  queue->list = list_create();;
  pthread_mutex_init(&queue->mutex, NULL);
  queue->length = 0;
}

void non_blocking_queue_destroy(NonBlockingQueueT* queue) {
  pthread_mutex_destroy(&queue->mutex);
  list_destroy(queue->list);
}

void non_blocking_queue_push(NonBlockingQueueT* queue, unsigned int value) {
  pthread_mutex_lock(&queue->mutex);
  list_append(queue->list, value);
  queue->length++;
  pthread_mutex_unlock(&queue->mutex);
}

int non_blocking_queue_pop(NonBlockingQueueT* queue, unsigned int* value) {
  pthread_mutex_lock(&queue->mutex);
  if(list_empty(queue->list)){
    pthread_mutex_unlock(&queue->mutex);
    return -1;
  }
  *value = list_pop_front(queue->list);
  queue->length--;
  pthread_mutex_unlock(&queue->mutex);

  return 0;
}

int non_blocking_queue_empty(NonBlockingQueueT* queue) {
  pthread_mutex_lock(&queue->mutex);        
  int is_empty = list_empty(queue->list);  
  pthread_mutex_unlock(&queue->mutex);   
  return is_empty;
}

int non_blocking_queue_length(NonBlockingQueueT* queue) {
  pthread_mutex_lock(&queue->mutex);        
  int length = queue->length;               
  pthread_mutex_unlock(&queue->mutex);      
  return length;
}
