/*
* @Author: karlosiric
* @Date:   2025-07-11 15:11:35
* @Last Modified by:   karlosiric
* @Last Modified time: 2025-07-14 10:02:28
*/

#include "../include/logger.h"
#include <cstdlib>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

static FILE *log_file = NULL;
static const char *log_file_path = NULL;

static const char *log_level_strings[] = {
    "INFO", "WARNING", "ERROR", "DEBUG"
};

static const char *log_activity_strings[] = {
    "REQUEST", "RESPONSE", "CONNECTION", "PARSE_ERROR",
    "MEMORY_ERROR", "NETWORK_ERROR", "SERVER_START",
    "SERVER_STOP"
};

static const char *log_direction_string[] = {
    "CLIENT->SERVER", "SERVER->CLIENT"
};

int logger_init(const char *log_file_path_param) {
    if (log_file_path_param == NULL) {
        fprintf(stderr, "Logger: NULL log file path provided\n");
        return (-1);
    }
    log_file_path = strdup(log_file_path_param);
    if (log_file_path == NULL) {
        fprintf(stderr, "Logger: Failed to allocate enough memory for file path\n");
        return (-1);
    }

    log_file = fopen(log_file_path, "a");
    if (log_file == NULL) {
        fprintf(stderr, "Logger: Failed to open log file: '%s': %s\n", 
                log_file_path, strerror(errno));
        free((void*)log_file_path);
        log_file_path = NULL;
        return (-1);
    }

    fprintf(log_file, "\n==== LOGGER STARTED ====\n");
    fflush(log_file);

    return (0);
}

void logger_close() {
    if (log_file) {
        fclose(log_file);
        log_file = NULL;
    }

    if (log_file_path) {
        free((void*)log_file_path);
        log_file_path = NULL;
    }
}


void log_message(e_log_level level, e_log_activity activity, const char *client_ip, int port, e_log_direction direction, const char *format, ...) {
    if (log_file == NULL) {
        fprintf(stderr, "Logger: Log file is NULL, trying to reopen the logger!\n");
        return;
    }
    /*
    // TODO: 1. Need to implement time stamping
             2. Need to 
    */


    if (level < 0 || level >= 4) {
        fprintf(stderr, "Invalid level parameter value, needs to be from (0 - 4)!");
        return;
    }

    if (activity < 0 || activity >= 8) {
        return;
    }

    if (direction < 0 || direction >= 3) {
        return;
    }

    time_t now;
    struct tm *timeinfo;
    char timestamp[32];

    time(&now);
    timeinfo = localtime(&now);
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", timeinfo);

    va_list args;
    va_start(args, format);

    fprintf(log_file, "[%s] [%s] [%s:%d] [%s] [%s]",
            timestamp,
            log_level_strings[level],
            client_ip ? client_ip : "Unknown IP",
            port,
            log_activity_strings[activity],
            log_direction_string[direction]);

    vfprintf(log_file, format, args);

    fprintf(log_file, "\n");

    va_end(args);

    fflush(log_file);
}

void log_http_request(const char *client_ip, int port, const char *request_line) {

}
