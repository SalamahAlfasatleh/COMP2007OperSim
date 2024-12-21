#include "blocking_queue.h"
#include "utilities.h"

#include <assert.h>

// Student : Salameh Alfasatleh ID: 20578169

#include <stdio.h>  

void test_blocking_queue_basic() {
  BlockingQueueT queue;
  blocking_queue_create(&queue);

  assert(blocking_queue_empty(&queue) == 1);
  assert(blocking_queue_length(&queue) == 0);

  blocking_queue_push(&queue, 42);
  assert(blocking_queue_empty(&queue) == 0);
  assert(blocking_queue_length(&queue) == 1);

  unsigned int value;
  assert(blocking_queue_pop(&queue, &value) == 0);
  assert(value == 42);
  assert(blocking_queue_empty(&queue) == 1);
  assert(blocking_queue_length(&queue) == 0);

  blocking_queue_destroy(&queue);
  printf("test_blocking_queue_basic passed!\n");
}

void test_blocking_queue_termination() {
  BlockingQueueT queue;
  blocking_queue_create(&queue);

  blocking_queue_terminate(&queue);

  unsigned int value;
  assert(blocking_queue_pop(&queue, &value) == -1);

  blocking_queue_destroy(&queue);
  printf("test_blocking_queue_termination passed!\n");
}

int main() {
  test_blocking_queue_basic();
  test_blocking_queue_termination();
  printf("All tests passed!\n");
  return 0;
}
