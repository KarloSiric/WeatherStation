/*
* @Author: karlosiric
* @Date:   2025-07-11 15:11:35
* @Last Modified by:   karlosiric
* @Last Modified time: 2025-07-14 14:14:53
*/

#include "../include/logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

static FILE *log_file = NULL;
static const char *log_file_path = NULL;

static int session_counter = 0;
int current_session_id = 0;

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

    if (log_file != NULL) {
        fprintf(stderr, "Logger: Already initialized\n");
        return (0);
    }


    if (log_file_path_param == NULL) {
        fprintf(stderr, "Logger: NULL log file path provided\n");
        return (-1);
    }

    /* TODO: Need to check if the directory exists and if it doesnt 
     *       Then we need to make one basically, for these we need some inc...
    */
    char *dir_path = strdup(log_file_path_param);
    char *last_slash = strrchr(dir_path, '/');
    if (last_slash != NULL) {
        *last_slash = '\0';

        // we then check if the dir exists
        struct stat st = {0};
        if (stat(dir_path, &st) == -1) {
            if (mkdir(dir_path, 0755) == -1) {
                fprintf(stderr, "Logger: Failed to create directory '%s': %s\n",
                        dir_path, strerror(errno));
                free(dir_path);
                return(-1);
            }
            printf("Logger: Created directory '%s'\n", dir_path);
        }

    }
    free(dir_path);

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

    session_counter++;
    current_session_id = session_counter;

    fprintf(log_file, "\n\n==== SERVER SESSION #%d STARTED ====\n\n", current_session_id);
    fflush(log_file);

    return (0);
}

void logger_close(void) {
    if (log_file) {
        fprintf(log_file, "\n\n=== SERVER SESSION #%d STOPPED ===\n\n", current_session_id);
        fclose(log_file);
        log_file = NULL;
    }

    if (log_file_path) {
        free((void*)log_file_path);
        log_file_path = NULL;
    }
}


void log_message(e_log_level level, e_log_activity activity, const char *client_ip, int port, e_log_direction direction, const char *format, ...) {

    /* TODO: Implementing a safety checkup system:
     * Checks if the file exists and everything is alright
     * If it doesnt then it needs to recreated because either the log file
     * or the log directory is missing and that needs to addressed so...
    */

    if (level < 0 || level >= 4) {
        fprintf(stderr, "Invalid level parameter value, needs to be from (0 to 3)!\n");
        return;
    }

    if (activity < 0 || activity >= 8) {
        fprintf(stderr, "Invalid activity parameter value!\n");
        return;
    }

    if (direction < 0 || direction >= 3) {
        fprintf(stderr, "Invalid direction parameter value!\n");
        return;
    }

    if (log_file == NULL) {
        fprintf(stderr, "Logger: Still no valid log file after recovery attempt.\n");
        return;
    }

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
    int need_recovery = 0;
    if (log_file == NULL) {
        fprintf(stderr, "Logger: Log file handle is NULL, attempting recovery...\n");
        need_recovery = 1;
    } else if (log_file_path != NULL) {
        // Now we check if the file still exists on the disk regardless if it maybe exists in memory
        if (access(log_file_path, F_OK) != 0) {
            fprintf(stderr, "Logger: Log file was deleted from the disk, attempting recovery ...\n");
            need_recovery = 1;
        }
    }

    /* TODO : Implementing the working recovering process
     * 
    */  

    if (need_recovery) {
        if (log_file_path == NULL) {
            fprintf(stderr, "Logger: No log file path available, using default 'logs/server.log'\n");
            log_file_path = strdup("logs/server.log");
            if (log_file_path == NULL) {
                fprintf(stderr, "Logger: Failed to allocate memory for the deafult log file path\n");
                return;
            }
        }

        if (log_file != NULL) {
            fclose(log_file);
            log_file = NULL;
        }

        char *dir_path = strdup(log_file_path);
        if (dir_path != NULL) {
            char *last_slash = strrchr(dir_path, '/');
            if (last_slash != NULL) {
                *last_slash = '\0';
                mkdir(dir_path, 0755);
            }
            free(dir_path);
        }


        // TODO: Now need to recreate the log file HANDLE properly
        log_file = fopen(log_file_path, "a");
        if (log_file == NULL) {
            fprintf(stderr, "Logger: Failed to recreate log file: '%s': %s\n",
                    log_file_path, strerror(errno));
            return;
        }

        // LOG RECOVERY MESSAGE
        fprintf(log_file, "\n\n==== LOGGER RECOVERED: SESSION #%d ====\n\n", current_session_id);
        fflush(log_file);
        fprintf(stderr, "Logger: Successfully recovered log file!\n");
    }

    /* TODO: Do another double check move just in case 
     * 
     *       To be sure everything is working properly for the final time
    */

    if (log_file == NULL) {
        fprintf(stderr, "Logger: Still no valid flag file after recovery attempt.\n");
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

    if (fflush(log_file) != 0) {
        fprintf(stderr, "Logger: Warning - Failed to flush log message\n");
    }
}

void log_http_request(const char *client_ip, int port, const char *request_line) {
    if (request_line == NULL) {
        log_message(LOG_ERROR, LOG_PARSE_ERROR, client_ip, port, LOG_CLIENT_TO_SERVER, "Received NULL request line");
        return;
    }

    log_message(LOG_INFO, LOG_REQUEST, client_ip, port, LOG_CLIENT_TO_SERVER, "HTTP Request: %s", request_line);
}  

void log_http_response(const char *client_ip, int port, const char *response, size_t bytes_sent) {
    if (response == NULL) {
        log_message(LOG_ERROR, LOG_RESPONSE, client_ip, port, LOG_SERVER_TO_CLIENT, "Sent NULL response back!");
        return;
    }

    log_message(LOG_INFO, LOG_RESPONSE, client_ip, port, LOG_SERVER_TO_CLIENT, "HTTP Request (%zu bytes): %s", bytes_sent, response);

    return;
}

void log_error(const char *client_ip, int port, const char *error_msg) {
    if (error_msg == NULL) {
        log_message(LOG_ERROR, LOG_PARSE_ERROR, client_ip, port, LOG_INTERNAL, "Unknown error occured");
        return;
    }

    log_message(LOG_ERROR, LOG_PARSE_ERROR, client_ip, port, LOG_INTERNAL, "Error: %s", error_msg); 
}
