/*
* @Author: karlosiric
* @Date:   2025-06-26 14:39:26
* @Last Modified by:   karlosiric
* @Last Modified time: 2025-06-28 22:08:13
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
        bytes_sent = send(client_fd, server_response, strlen(server_response), 0);
        if (bytes_sent < 0) {
            printf("Failed to send the response: %s\n", strerror(errno));
            close(client_fd);
            free(buffer); 
            continue;
        } else if ((size_t)bytes_sent < strlen(server_response)) {
            printf("Warning: Only sent %zu out of %zu data bytes\n", bytes_sent, strlen(server_response));

        } else {
            printf("Successfully sent all of the %zu bytes of data!", bytes_sent);
            close(client_fd);
            free(buffer);
        }
    }

    printf("Server closing down ... \n");

    return (0);
}
