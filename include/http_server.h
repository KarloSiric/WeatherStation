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

#define HTTP_PORT           8080
#define MAX_CONNECTIONS     10
#define MAX_BUFFER_SIZE      8192

int start_http_server(void);


#endif
