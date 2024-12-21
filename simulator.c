#include "simulator.h"
#include "list.h"
#include "non_blocking_queue.h"
#include "blocking_queue.h"
#include "utilities.h"
#include "logger.h"

void simulator_start(int thread_count, int max_processes) {
}

void simulator_stop() {
}

ProcessIdT simulator_create_process(EvaluatorCodeT const code) {
  ProcessIdT pid = 0;
  return pid;
}

void simulator_wait(ProcessIdT pid) {
}

void simulator_kill(ProcessIdT pid) {
}

void simulator_event() {
}
