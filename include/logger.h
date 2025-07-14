#ifndef LOGGER_H
#define LOGGER_H 

#include <stdio.h>
#include <time.h>
#include <stdarg.h>

typedef enum {
    LOG_INFO,
    LOG_WARNING,
    LOG_ERROR,
    LOG_DEBUG
} e_log_level;

typedef enum {
    LOG_REQUEST,
    LOG_RESPONSE,
    LOG_CONNECTION,
    LOG_PARSE_ERROR,
    LOG_MEMORY_ERROR,
    LOG_NETWORK_ERROR,
    LOG_SERVER_START,
    LOG_SERVER_STOP
} e_log_activity;

typedef enum {
    LOG_CLIENT_TO_SERVER,
    LOG_SERVER_TO_CLIENT,
    LOG_INTERNAL
} e_log_direction;

extern int current_session_id;

int logger_init(const char *log_file_path);
void logger_close(void);
void log_message(e_log_level level, e_log_activity activity,
                const char *client_ip, int port, e_log_direction direction,
                const char *format, ...);

void log_http_request(const char *client_ip, int port, const char *request_line);
void log_http_response(const char *client_ip, int port, const char *response, size_t bytes_sent);
void log_error(const char *client_ip, int port, const char *error_msg);

#endif
