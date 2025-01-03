#include "environment.h"
#include "simulator.h"
#include "utilities.h"
#include "evaluator.h"
#include "list.h"
#include "logger.h"
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>

// Thread management structures
pthread_t* environment_threads = NULL;
unsigned int environment_thread_count = 0;

// Routine for infinite-running processes
void* infinite_routine(void* arg) {
    unsigned int iterations = *((unsigned int*)arg);
    unsigned int batch_size = *((unsigned int*)(arg + sizeof(unsigned int)));
    free(arg);

    char log_message[128];
    sprintf(log_message, "Infinite routine started with %u iterations and batch size %u.", iterations, batch_size);
    logger_write(log_message);

    for (unsigned int i = 0; i < iterations; i++) {
        ProcessIdT pids[batch_size];

        // Create batch_size processes
        for (unsigned int j = 0; j < batch_size; j++) {
            EvaluatorCodeT code = evaluator_infinite_loop; // Processes that run indefinitely
            pids[j] = simulator_create_process(code);

            // Log process creation
            sprintf(log_message, "Created process %u in iteration %u.", pids[j], i);
            logger_write(log_message);
        }

        // Kill the processes after they are created
        for (unsigned int j = 0; j < batch_size; j++) {
            simulator_kill(pids[j]);

            // Wait for the process to terminate
            simulator_wait(pids[j]);

            // Log process termination
            sprintf(log_message, "Process %u killed.", pids[j]);
            logger_write(log_message);
        }
    }

    sprintf(log_message, "Infinite routine finished.");
    logger_write(log_message);

    return NULL;
}


// Start environment with thread_count threads
void environment_start(unsigned int thread_count,
                       unsigned int iterations,
                       unsigned int batch_size) {
    environment_thread_count = thread_count;
    environment_threads = (pthread_t*)malloc(sizeof(pthread_t) * thread_count);

    if (!environment_threads) {
        fprintf(stderr, "Error: Unable to allocate threads for environment.\n");
        exit(EXIT_FAILURE);
    }

    for (unsigned int i = 0; i < thread_count; i++) {
        unsigned int* thread_args = malloc(2 * sizeof(unsigned int));
        thread_args[0] = iterations;
        thread_args[1] = batch_size;

        // Log thread creation
        char log_message[128];
        sprintf(log_message, "Creating thread %u for infinite routine.", i);
        logger_write(log_message);

        if (pthread_create(&environment_threads[i], NULL, infinite_routine, thread_args) != 0) {
            fprintf(stderr, "Error: Unable to create environment thread %u\n", i);
            exit(EXIT_FAILURE);
        }
    }
}

// Stop environment and clean up threads
void environment_stop() {
    for (unsigned int i = 0; i < environment_thread_count; i++) {
        // Log when waiting for a thread to finish
        char log_message[128];
        sprintf(log_message, "Joining thread %u.", i);
        logger_write(log_message);

        pthread_join(environment_threads[i], NULL);
    }

    free(environment_threads);
    environment_threads = NULL;
    environment_thread_count = 0;
}
