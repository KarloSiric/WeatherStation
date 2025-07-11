#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H 

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/tcp.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define HTTP_PORT            8080
#define MAX_CONNECTIONS      10
#define MAX_BUFFER_SIZE      8192

extern volatile int keep_running;
typedef enum {
    ROUTE_HOME,
    ROUTE_WEATHER,
    ROUTE_API_WEATHER,
    ROUTE_NOT_FOUND
} e_routing;

typedef struct {
    char city[256];
    char latitude[16];
    char longitude[16];
    int has_city;
    int has_coordinates;
} s_query_params;

int start_http_server(void);
int parse_http_request(const char *request, char *method, char *path);
int parse_query_request(char *path, s_query_params *params);
e_routing determine_route(const char *path);
int url_decoding(char *dst, const char *src);

#endif
