#include "simulator.h"
#include "list.h"
#include "logger.h"
#include "evaluator.h"
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

ListT* blocked_queue = NULL;

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
    blocked_queue = list_create();  // Initialize blocked queue

    if (!processes || !task_queue || !blocked_queue) {
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

        // Stop if there are no tasks left to process
        if (list_empty(task_queue) && list_empty(blocked_queue)) {
            pthread_mutex_unlock(&process_mutex);
            break;  // Exit the loop if no tasks are left
        }

        ProcessIdT task_id = 0;
        if (!list_empty(task_queue)) {
            task_id = list_pop_front(task_queue);
        } else if (!list_empty(blocked_queue)) {
            task_id = list_pop_front(blocked_queue); // Get a blocked process
        }

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
        } else if (result.reason == reason_blocked) {
            process->state = blocked;
            list_append(blocked_queue, task_id); // Add to blocked queue
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

    // Ensure all worker threads stop properly after killing all processes
    for (int i = 0; i < total_threads; i++) {
        pthread_join(worker_threads[i], NULL);
    }

    list_destroy(task_queue);
    list_destroy(blocked_queue);  // Destroy blocked queue
    free(processes);
    free(worker_threads);

    pthread_mutex_destroy(&process_mutex);
    pthread_cond_destroy(&process_condition);

    char log_message[128];
    sprintf(log_message, "Simulator has stopped.");
    logger_write(log_message);
}



// Terminate a specific process
void simulator_kill(ProcessIdT pid) {
    pthread_mutex_lock(&process_mutex);

    // Log the kill request
    char log_message[128];
    sprintf(log_message, "Requesting to kill process %d", pid);
    logger_write(log_message);

    // Move the process to the terminated state
    processes[pid - 1].state = terminated;

    // Log the termination
    sprintf(log_message, "Process %d has been moved to terminated state.", pid);
    logger_write(log_message);

    // Now remove the process from both task_queue and blocked_queue
    struct List* node_to_remove = list_find_first(task_queue, pid);
    if (node_to_remove) {
        list_remove(task_queue, node_to_remove);
        sprintf(log_message, "Process %d removed from task queue.", pid);
        logger_write(log_message);
    }

    node_to_remove = list_find_first(blocked_queue, pid);
    if (node_to_remove) {
        list_remove(blocked_queue, node_to_remove);
        sprintf(log_message, "Process %d removed from blocked queue.", pid);
        logger_write(log_message);
    }

    // Broadcast to unblock waiting threads
    pthread_cond_broadcast(&process_condition);

    pthread_mutex_unlock(&process_mutex);
}



void simulator_event() {
    if (!list_empty(blocked_queue)) {
        ProcessIdT pid = list_pop_front(blocked_queue);  // Move the front blocked process
        ProcessControlBlock* pcb = &processes[pid - 1];

        // Move to ready queue
        pcb->state = ready;
        list_append(task_queue, pid);

        char log_message[128];
        sprintf(log_message, "Process %d moved to ready queue from blocked.", pid);
        logger_write(log_message);
    }
}
