#include "non_blocking_queue.h"
#include "utilities.h"

#include <assert.h>

// Student : Salameh Alfasatleh ID: 20578169
#include <stdio.h>


void test_non_blocking_queue() {
  NonBlockingQueueT queue;
  non_blocking_queue_create(&queue);

  assert(non_blocking_queue_empty(&queue));
  assert(non_blocking_queue_length(&queue) == 0);

  non_blocking_queue_push(&queue, 42);
  assert(!non_blocking_queue_empty(&queue));
  assert(non_blocking_queue_length(&queue) == 1);

  unsigned int value;
  assert(non_blocking_queue_pop(&queue, &value) == 0);
  assert(value == 42);
  assert(non_blocking_queue_empty(&queue));
  assert(non_blocking_queue_length(&queue) == 0);

  non_blocking_queue_destroy(&queue);
}

int main() {
  test_non_blocking_queue();
  printf("All tests passed!\n");
  return 0;
}
