/*
* @Author: karlosiric
* @Date:   2025-07-11 15:11:35
* @Last Modified by:   karlosiric
* @Last Modified time: 2025-07-14 15:20:38
*/

#include "../include/logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

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

/*
 * Improved logger functions with better formatting and readability
 */

// Enhanced logger_init function
int logger_init(const char *log_file_path_param) {
    if (log_file != NULL) {
        fprintf(stderr, "Logger: Already initialized\n");
        return (0);
    }

    if (log_file_path_param == NULL) {
        fprintf(stderr, "Logger: NULL log file path provided\n");
        return (-1);
    }

    /* Directory creation code stays the same... */
    char *dir_path = strdup(log_file_path_param);
    char *last_slash = strrchr(dir_path, '/');
    if (last_slash != NULL) {
        *last_slash = '\0';
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

    // Enhanced startup header
    time_t now;
    struct tm *timeinfo;
    char start_timestamp[32];

    time(&now);
    timeinfo = localtime(&now);
    strftime(start_timestamp, sizeof(start_timestamp), "%Y-%m-%d %H:%M:%S", timeinfo);

    fprintf(log_file, "\n\n");
    fprintf(log_file, "================================================================================\n");
    fprintf(log_file, "                        WEATHER STATION SERVER STARTED\n");
    fprintf(log_file, "                             %s\n", start_timestamp);
    fprintf(log_file, "================================================================================\n");
    fprintf(log_file, "Server Configuration:\n");
    fprintf(log_file, "  ▶ Port: 8080\n");
    fprintf(log_file, "  ▶ Log File: %s\n", log_file_path_param);
    fprintf(log_file, "  ▶ Max Connections: 10\n");
    fprintf(log_file, "  ▶ Buffer Size: 8192 bytes\n");
    fprintf(log_file, "--------------------------------------------------------------------------------\n\n");
    
    fflush(log_file);
    return (0);
}

// Enhanced logger_close function
void logger_close(void) {
    if (log_file) {
        time_t now;
        struct tm *timeinfo;
        char stop_timestamp[32];

        time(&now);
        timeinfo = localtime(&now);
        strftime(stop_timestamp, sizeof(stop_timestamp), "%Y-%m-%d %H:%M:%S", timeinfo);

        fprintf(log_file, "\n");
        fprintf(log_file, "--------------------------------------------------------------------------------\n");
        fprintf(log_file, "                        WEATHER STATION SERVER STOPPED\n");
        fprintf(log_file, "                             %s\n", stop_timestamp);
        fprintf(log_file, "================================================================================\n\n");
        
        fclose(log_file);
        log_file = NULL;
    }

    if (log_file_path) {
        free((void*)log_file_path);
        log_file_path = NULL;
    }
}

// Enhanced log_http_request function
void log_http_request(const char *client_ip, int port, const char *request_line) {
    if (request_line == NULL) {
        log_message(LOG_ERROR, LOG_PARSE_ERROR, client_ip, port, LOG_CLIENT_TO_SERVER, 
                   "Received NULL request line");
        return;
    }

    // Parse the request line to extract method, path, and protocol
    char method[16] = {0};
    char path[256] = {0};
    char protocol[16] = {0};
    
    if (sscanf(request_line, "%15s %255s %15s", method, path, protocol) >= 2) {
        time_t now;
        struct tm *timeinfo;
        char timestamp[32];

        time(&now);
        timeinfo = localtime(&now);
        strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", timeinfo);

        fprintf(log_file, "[%s] [📥 INCOMING REQUEST] %s:%d\n", timestamp, client_ip, port);
        fprintf(log_file, "    ▶ Method: %s | Path: %s", method, path);
        if (strlen(protocol) > 0) {
            fprintf(log_file, " | Protocol: %s", protocol);
        }
        fprintf(log_file, "\n");
        
        // Extract User-Agent if present
        const char *user_agent = strstr(request_line, "User-Agent:");
        if (user_agent) {
            const char *start = user_agent + 11; // Skip "User-Agent:"
            while (*start == ' ') start++; // Skip spaces
            const char *end = strchr(start, '\r');
            if (!end) end = strchr(start, '\n');
            if (end) {
                int len = end - start;
                if (len > 0 && len < 100) {
                    fprintf(log_file, "    ▶ User-Agent: %.*s\n", len, start);
                }
            }
        }
        fprintf(log_file, "\n");
    } else {
        // Fallback to original format if parsing fails
        log_message(LOG_INFO, LOG_REQUEST, client_ip, port, LOG_CLIENT_TO_SERVER, 
                   "HTTP Request: %s", request_line);
    }

    fflush(log_file);
}

// Enhanced log_http_response function  
void log_http_response(const char *client_ip, int port, const char *response, size_t bytes_sent) {
    if (response == NULL) {
        log_message(LOG_ERROR, LOG_RESPONSE, client_ip, port, LOG_SERVER_TO_CLIENT, 
                   "Sent NULL response back!");
        return;
    }

    time_t now;
    struct tm *timeinfo;
    char timestamp[32];

    time(&now);
    timeinfo = localtime(&now);
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", timeinfo);

    // Extract status code from response
    char status_code[10] = {0};
    if (sscanf(response, "HTTP/%*s %9s", status_code) == 1) {
        fprintf(log_file, "[%s] [📤 RESPONSE SENT] %s:%d\n", timestamp, client_ip, port);
        fprintf(log_file, "    ▶ Status: %s | Bytes: %zu\n", status_code, bytes_sent);
        
        // Add status description
        if (strncmp(status_code, "200", 3) == 0) {
            fprintf(log_file, "    ▶ Result: ✅ Success\n");
        } else if (strncmp(status_code, "404", 3) == 0) {
            fprintf(log_file, "    ▶ Result: ❌ Not Found\n");
        } else if (strncmp(status_code, "500", 3) == 0) {
            fprintf(log_file, "    ▶ Result: ⚠️  Server Error\n");
        }
    } else {
        fprintf(log_file, "[%s] [📤 RESPONSE SENT] %s:%d\n", timestamp, client_ip, port);
        fprintf(log_file, "    ▶ Bytes: %zu\n", bytes_sent);
    }
    fprintf(log_file, "\n");

    fflush(log_file);
}

// Enhanced log_message function (the main logging function)
void log_message(e_log_level level, e_log_activity activity, const char *client_ip, int port, e_log_direction direction, const char *format, ...) {
    
    // Validate parameters first
    if (level < 0 || level >= 4) {
        fprintf(stderr, "Invalid level parameter value, needs to be from (0 - 3)!\n");
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

    /* Handle NULL log_file OR missing file on disk */
    int need_recovery = 0;
    
    if (log_file == NULL) {
        fprintf(stderr, "Logger: Log file handle is NULL, attempting recovery...\n");
        need_recovery = 1;
    } else if (log_file_path != NULL) {
        // Check if file still exists on disk
        if (access(log_file_path, F_OK) != 0) {
            fprintf(stderr, "Logger: Log file was deleted from disk, attempting recovery...\n");
            need_recovery = 1;
        }
    }

    /* Recovery process */
    if (need_recovery) {
        if (log_file_path == NULL) {
            fprintf(stderr, "Logger: Cannot recover - no file path available\n");
            return;
        }

        // Close current handle if it exists
        if (log_file != NULL) {
            fclose(log_file);
            log_file = NULL;
        }

        // Ensure directory exists
        char *dir_path = strdup(log_file_path);
        if (dir_path != NULL) {
            char *last_slash = strrchr(dir_path, '/');
            if (last_slash != NULL) {
                *last_slash = '\0';
                mkdir(dir_path, 0755);
            }
            free(dir_path);
        }

        // Recreate the log file
        log_file = fopen(log_file_path, "a");
        if (log_file == NULL) {
            fprintf(stderr, "Logger: Failed to recreate log file '%s': %s\n", 
                    log_file_path, strerror(errno));
            return;
        }

        // Log recovery message with nice formatting
        time_t now;
        struct tm *timeinfo;
        char recovery_timestamp[32];

        time(&now);
        timeinfo = localtime(&now);
        strftime(recovery_timestamp, sizeof(recovery_timestamp), "%Y-%m-%d %H:%M:%S", recovery_timestamp);

        fprintf(log_file, "\n🔄 LOGGER RECOVERED: %s 🔄\n", recovery_timestamp);
        fprintf(log_file, "    ▶ Log file was recreated after being deleted\n\n");
        fflush(log_file);
        fprintf(stderr, "Logger: Successfully recovered log file\n");
    }

    // At this point, log_file should be valid, but double-check
    if (log_file == NULL) {
        fprintf(stderr, "Logger: Still no valid log file after recovery attempt\n");
        return;
    }

    // Generate timestamp
    time_t now;
    struct tm *timeinfo;
    char timestamp[32];

    time(&now);
    timeinfo = localtime(&now);
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", timeinfo);

    // Enhanced log level and activity icons/descriptions
    const char *level_icons[] = {"ℹ️", "⚠️", "❌", "🔍"};
    const char *level_names[] = {"INFO", "WARNING", "ERROR", "DEBUG"};
    
    const char *activity_icons[] = {
        "📥", "📤", "🔗", "❌", "💾", "🌐", "🚀", "🛑"
    };
    const char *activity_names[] = {
        "REQUEST", "RESPONSE", "CONNECTION", "PARSE_ERROR",
        "MEMORY_ERROR", "NETWORK_ERROR", "SERVER_START", "SERVER_STOP"
    };

    const char *direction_arrows[] = {"←", "→", "⚡"};
    const char *direction_names[] = {"CLIENT→SERVER", "SERVER→CLIENT", "INTERNAL"};

    // Write the enhanced log message
    va_list args;
    va_start(args, format);

    fprintf(log_file, "[%s] [%s %s] [%s %s:%d] [%s %s] ",
            timestamp,
            level_icons[level], level_names[level],
            direction_arrows[direction], client_ip ? client_ip : "Unknown", port,
            activity_icons[activity], activity_names[activity]);

    vfprintf(log_file, format, args);
    fprintf(log_file, "\n");

    va_end(args);

    // Make sure it's written to disk
    if (fflush(log_file) != 0) {
        fprintf(stderr, "Logger: Warning - failed to flush log message\n");
    }
}

// Enhanced log_error function
void log_error(const char *client_ip, int port, const char *error_msg) {
    if (error_msg == NULL) {
        log_message(LOG_ERROR, LOG_PARSE_ERROR, client_ip, port, LOG_INTERNAL, 
                   "Unknown error occurred");
        return;
    }

    time_t now;
    struct tm *timeinfo;
    char timestamp[32];

    time(&now);
    timeinfo = localtime(&now);
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", timeinfo);

    fprintf(log_file, "[%s] [❌ ERROR] %s:%d\n", timestamp, client_ip, port);
    fprintf(log_file, "    ▶ Details: %s\n\n", error_msg);

    fflush(log_file);
}
