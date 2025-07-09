/*
* @Author: karlosiric
* @Date:   2025-06-26 14:39:26
* @Last Modified by:   karlosiric
* @Last Modified time: 2025-07-09 14:57:22
*/


#include "../include/http_server.h"
#include <signal.h>
#include <stdio.h>
#include <string.h>

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
        char *server_response;
        printf("=== Received HTTP Request ===\n%s\n=== End Request ===\n", buffer);
        char method[16];
        char path[256];
        char city_name[256];
        int parse_result = parse_http_request(buffer, method, path);
        int query_result = parse_query_request(path, city_name);

        e_routing routing_type = determine_route(path);

        switch(routing_type) {
            case ROUTE_HOME:
                server_response = "HTTP/1.0 200 OK\r\n\r\nWelcome to the Weather Station!";
                break;
            case ROUTE_API_WEATHER:
                server_response = "HTTP/1.0 200 OK\r\n\r\nAPI Endpoint Working!";
                break;
            case ROUTE_WEATHER:
                server_response = "HTTP/1.0 200 OK\r\n\r\nWeather Station Page!";
                break;
            default:
                server_response = "HTTP/1.0 404 Not Found\r\n\r\n404 - Page Not Found!";
                break;
        }
        if (query_result == 0) {
            printf("Parsed city from the URL: '%s'\n", city_name);
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

int parse_query_request(char *path, char *city_name) {
    if (path == NULL) {
        fprintf(stderr, "Error the path pointer is NULL!");
        return (-1);
    }

    int question_mark_index = -1;
    char *city_buffer;
    city_buffer = (char *)malloc(256 * sizeof(char));

    for (int i = 0; path[i] != '\0'; i++) {
        if (path[i] == '?') {
            question_mark_index = i;
            break;
        }
    }

    int buffer_index = 0;
    for (int j = question_mark_index + 1; j < (int)strlen(path); j++) {
        city_buffer[j - question_mark_index - 1] = path[j];   
        buffer_index++;
    }

    city_buffer[buffer_index] = '\0';
    

    int equals_index = -1;

    for (int i = 0; *city_buffer != '\0'; i++) {
        if (city_buffer[i] == '=') {
            equals_index = i;
            break;
        }
    }
    int city_index = 0;
    for (int k = equals_index + 1; k < (int) strlen(city_buffer); k++) {
        city_name[k - equals_index - 1] = city_buffer[k];
        city_index++;
    }
    city_name[city_index] = '\0';

    free(city_buffer);
    return (0);
}

e_routing determine_route(const char *path) {
    if (strcmp(path, "/") == 0) {
        return ROUTE_HOME;
    } 

    if (strncmp(path, "/api/weather", 12) == 0) {
        return ROUTE_API_WEATHER;
    }

    if (strcmp(path, "/weather") == 0) {
        return ROUTE_WEATHER;
    }

    return ROUTE_NOT_FOUND;
}
