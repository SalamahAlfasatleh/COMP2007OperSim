#include "simulator.h"
#include "list.h"
#include "logger.h"
#include "evaluator.h"
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

// Data structures for thread and process management
pthread_t* worker_threads = NULL;  
int total_threads = 0;            

ProcessControlBlock* processes = NULL;  
ListT* task_queue = NULL;              

pthread_mutex_t process_mutex;         
pthread_cond_t process_condition;      

unsigned int max_tasks;                
bool simulator_active = true;          

void* simulator_routine(void* arg);

// Initialize simulator resources
void simulator_start(int threads, int max_processes) {
    total_threads = threads;
    simulator_active = true;

    // Allocate and initialize resources
    processes = (ProcessControlBlock*)malloc(sizeof(ProcessControlBlock) * max_processes);
    task_queue = list_create();

    if (!processes || !task_queue) {
        fprintf(stderr, "Error: Unable to allocate resources for simulator.\n");
        exit(EXIT_FAILURE);
    }

    for (unsigned int i = 0; i < max_processes; i++) {
        processes[i].pid = i + 1;  
        processes[i].state = unallocated;
    }
    max_tasks = max_processes;

    pthread_mutex_init(&process_mutex, NULL);
    pthread_cond_init(&process_condition, NULL);

    worker_threads = (pthread_t*)malloc(sizeof(pthread_t) * total_threads);
    if (!worker_threads) {
        fprintf(stderr, "Error: Unable to allocate memory for threads.\n");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < total_threads; i++) {
        int* thread_id = (int*)malloc(sizeof(int));
        *thread_id = i;
        if (pthread_create(&worker_threads[i], NULL, simulator_routine, thread_id) != 0) {
            fprintf(stderr, "Error: Failed to create thread %d\n", i);
            exit(EXIT_FAILURE);
        }
    }
}

// Main worker thread function
void* simulator_routine(void* arg) {
    int thread_id = *(int*)arg;
    free(arg);

    char log_buffer[128];
    sprintf(log_buffer, "Thread %d started.", thread_id);
    logger_write(log_buffer);

    while (simulator_active) {
        pthread_mutex_lock(&process_mutex);

        if (list_empty(task_queue)) {
            pthread_mutex_unlock(&process_mutex);
            usleep(1000);  
            continue;
        }

        ProcessIdT task_id = list_pop_front(task_queue);
        ProcessControlBlock* process = &processes[task_id - 1];
        process->state = running;

        pthread_mutex_unlock(&process_mutex);

        EvaluatorResultT result = evaluator_evaluate(process->code, process->PC);
        process->PC = result.PC;

        pthread_mutex_lock(&process_mutex);

        if (result.reason == reason_terminated) {
            process->state = terminated;
            pthread_cond_broadcast(&process_condition);
        } else if (result.reason == reason_timeslice_ended) {
            process->state = ready;
            list_append(task_queue, task_id);
        }

        pthread_mutex_unlock(&process_mutex);
    }

    sprintf(log_buffer, "Thread %d stopping.", thread_id);
    logger_write(log_buffer);
    return NULL;
}

// Create a new process
ProcessIdT simulator_create_process(EvaluatorCodeT const code) {
    pthread_mutex_lock(&process_mutex);

    for (unsigned int i = 0; i < max_tasks; i++) {
        if (processes[i].state == unallocated) {
            ProcessControlBlock* process = &processes[i];
            process->pid = i + 1;
            process->state = ready;
            process->code = code;
            process->PC = 0;

            list_append(task_queue, process->pid);

            pthread_mutex_unlock(&process_mutex);
            return process->pid;
        }
    }

    pthread_mutex_unlock(&process_mutex);
    return 0;
}

// Wait for a process to complete
void simulator_wait(ProcessIdT pid) {
    pthread_mutex_lock(&process_mutex);

    while (processes[pid - 1].state != terminated) {
        pthread_cond_wait(&process_condition, &process_mutex);
    }

    pthread_mutex_unlock(&process_mutex);
}

// Stop the simulator and clean up resources
void simulator_stop() {
    simulator_active = false;

    for (int i = 0; i < total_threads; i++) {
        pthread_join(worker_threads[i], NULL);
    }

    list_destroy(task_queue);
    free(processes);
    free(worker_threads);

    pthread_mutex_destroy(&process_mutex);
    pthread_cond_destroy(&process_condition);
}

// Terminate a specific process
void simulator_kill(ProcessIdT pid) {
    pthread_mutex_lock(&process_mutex);
    processes[pid - 1].state = terminated;
    pthread_cond_broadcast(&process_condition);
    pthread_mutex_unlock(&process_mutex);
}

void simulator_event() {
}
