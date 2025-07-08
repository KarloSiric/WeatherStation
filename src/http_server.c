/*
* @Author: karlosiric
* @Date:   2025-06-26 14:39:26
* @Last Modified by:   karlosiric
* @Last Modified time: 2025-07-08 13:40:29
*/


#include "../include/http_server.h"
#include <signal.h>
#include <stdio.h>

int start_http_server(void) {

    int socket_fd, bind_result, client_fd;
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd == -1) {
        printf("Error creating socket: %s\n", strerror(errno));
        return (-1);
    }

    struct sockaddr_in server_address;
    struct sockaddr_in client_address;
    socklen_t client_addr_len = sizeof(client_address);

    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;                     // for internet protocol version 4
    server_address.sin_port = htons(HTTP_PORT);              // this is to convert the port number to network byte order (Big Endian network byte order).
    server_address.sin_addr.s_addr = INADDR_ANY;             // this is to bind the socket to all available interfaces on the machine.

    bind_result = bind(socket_fd, (struct sockaddr *)&server_address, sizeof(server_address));

    if (bind_result == -1) {
        printf("Error binding the socket: %s\n", strerror(errno));
        close(socket_fd);
        return (-2);
    }

    if (listen(socket_fd, MAX_CONNECTIONS) == -1) {
        printf("Error listening on the socket: %s\n", strerror(errno));
        close(socket_fd);
        return (-3);
    }

    printf("HTTP server started on port: %d\n", HTTP_PORT);
    
    ssize_t client_message;
    ssize_t bytes_sent;
    char *buffer;
    while(keep_running) {
        client_fd = accept(socket_fd, (struct sockaddr *)&client_address, &client_addr_len);
        if (client_fd == -1) {
            printf("Error accepting connections on the socket: %s\n", strerror(errno));
            close(socket_fd);
            return (-4);
        }
        buffer = (char *)malloc(MAX_BUFFER_SIZE * sizeof(char));
        client_message = recv(client_fd, buffer,  MAX_BUFFER_SIZE * sizeof(char), 0);
        if (client_message <= 0) {
            printf("Failed to read clien't message, received: %zu bytes of data!", client_message);
            free(buffer);
            close(client_fd);
            continue;
        }
        buffer[client_message] = '\0';  
        char *server_response = "HTTP/1.0 200 OK\r\n\r\nHello World!";
        printf("=== Received HTTP Request ===\n%s\n=== End Request ===\n", buffer);
        char method[16];
        char path[256];
        int parse_result = parse_http_request(buffer, method, path);

        if (parse_result == 0) {
            printf("Parsed Method: %s\n", method);
            printf("Parsed Path: %s\n", path);
        } else {
            printf("Error parsing request\n");
        }

        bytes_sent = send(client_fd, server_response, strlen(server_response), 0);
        if (bytes_sent < 0) {
            printf("Failed to send the response: %s\n", strerror(errno));
            close(client_fd);
            free(buffer); 
            continue;
        } else if ((size_t)bytes_sent < strlen(server_response)) {
            printf("Warning: Only sent %zu out of %zu data bytes\n", bytes_sent, strlen(server_response));

        } else {
            printf("Successfully sent all of the %zu bytes of data!\n", bytes_sent);
            close(client_fd);
            free(buffer);
        }
    }

    printf("Server closing down ... \n");

    return (0);
}

int parse_http_request(const char *request, char *method, char *path) {
    if (request == NULL || *request == '\0' || path == NULL || method == NULL) {
        fprintf(stderr, "Client HTTP request message is empty, nothing to parse!\n");
        return (-1);
    }

    int first_space = -1;
    for (int i = 0; request[i] != '\0'; i++) {
        if (request[i] == ' ') {
            first_space = i;
            break;
        }
    }

    if (first_space == -1) {
        return (-1);
    }

    // Now we need to copy everything here
    for (int j = 0; j < first_space; j++) {
        method[j] = request[j];
    }

    method[first_space] = '\0';

    int second_space = -1;
    // Now we need to find the other space
    for (int i = first_space + 1; request[i] != '\0'; i++) {
        if (request[i] == ' ') {
            second_space = i;
            break;
        }
    }

    if (second_space == -1) {
        return (-1);
    }

    for (int j = first_space + 1; j < second_space; j++) {
        path[j - first_space - 1] = request[j];
    }
    path[second_space - first_space - 1] = '\0';

        
    return 0;
}


