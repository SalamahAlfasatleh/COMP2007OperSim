#include "logger.h"
#include "utilities.h"

// Student : Salameh Alfasatleh ID: 20578169
#include <stdio.h>
#include <time.h>
#include <pthread.h>

static int message_c = 0;
static pthread_mutex_t logger_mutex;

void logger_start() {
	pthread_mutex_init(&logger_mutex, NULL);
}

void logger_stop() {
	pthread_mutex_destroy(&logger_mutex);
}

void logger_write(char const* message) {
	pthread_mutex_lock(&logger_mutex);

	time_t raw_time;
	struct tm* time_info;
	char time_buffer[9];

	time(&raw_time);
	time_info = localtime(&raw_time);
	strftime(time_buffer, sizeof(time_buffer), "%H:%M:%S", time_info);

	printf("%d : %s : %s\n", message_c, time_buffer, message);

	message_c++;

	pthread_mutex_unlock(&logger_mutex);
}
