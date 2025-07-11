/*
* @Author: karlosiric
* @Date:   2025-07-11 15:11:35
* @Last Modified by:   karlosiric
* @Last Modified time: 2025-07-11 15:39:08
*/

#include "../include/logger.h"
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

