#include "event_source.h"
#include "utilities.h"
#include "simulator.h"
#include <pthread.h>
#include <unistd.h>
#include <stdbool.h>  // Include for bool, true, false
#include <stdio.h>    // Include for fprintf, stderr

static pthread_t event_thread;
static bool event_source_active = false;

// Function to generate events at regular intervals
void* event_source_routine(void* arg) {
    useconds_t interval = *((useconds_t*)arg);
    free(arg);

    while (event_source_active) {
        usleep(interval);  // Wait for the specified interval
        simulator_event();  // Trigger the event in the simulator
    }

    return NULL;
}

// Start the event source to call simulator_event every interval microseconds
void event_source_start(useconds_t interval) {
    if (event_source_active) {
        return;  // If the event source is already active, do nothing
    }

    event_source_active = true;

    int* interval_ptr = malloc(sizeof(int));
    *interval_ptr = interval;

    // Create the event thread
    if (pthread_create(&event_thread, NULL, event_source_routine, interval_ptr) != 0) {
        fprintf(stderr, "Error: Failed to create event source thread\n");
        exit(EXIT_FAILURE);
    }
}

// Stop the event source and clean up resources
void event_source_stop() {
    event_source_active = false;

    if (event_thread != 0) {
        pthread_join(event_thread, NULL);  // Wait for the event thread to finish
    }
}
