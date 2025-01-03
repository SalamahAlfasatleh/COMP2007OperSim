#ifndef _SIMULATOR_H_
#define _SIMULATOR_H_

#include "evaluator.h"

// Student: Salameh Alfasatleh ID: 20578169

typedef unsigned int ProcessIdT;

typedef enum ProcessState {
    unallocated,
    ready,
    running,
    blocked,
    terminated
} ProcessStateT;

typedef struct {
    ProcessIdT pid;
    ProcessStateT state;
    EvaluatorCodeT code;  // Code the process will execute
    unsigned int PC;      // Program Counter to track execution state
} ProcessControlBlock;

void simulator_start(int threads, int max_processes);
void simulator_stop();

ProcessIdT simulator_create_process(EvaluatorCodeT const code);
void simulator_wait(ProcessIdT pid);
void simulator_kill(ProcessIdT pid);
void simulator_event();

#endif
