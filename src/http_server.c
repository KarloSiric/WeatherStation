/*
* @Author: karlosiric
* @Date:   2025-06-26 14:39:26
* @Last Modified by:   karlosiric
* @Last Modified time: 2025-07-11 15:46:48
*/


#include "../include/http_server.h"
#include "../include/logger.h"
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
        } else {
            printf("Successfully processed the client's HTTP request, containing %zu bytes of data!\n\n", client_message);
        }

        buffer[client_message] = '\0';  
        char *server_response;
        printf("=== Received HTTP Request ===\n%s\n=== End Request ===\n\n", buffer);
        char method[16];
        char path[256];
        s_query_params param = {0};
        int parse_result = parse_http_request(buffer, method, path);
        int query_result = parse_query_request(path, &param);

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
            printf("Parsed city from the URL: '%s'\n", param.city);
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
            printf("Successfully sent all of the %zu bytes of data back to the Client!\n\n", bytes_sent);
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

int parse_query_request(char *path, s_query_params *params) {
    // TODO: Need to extract the query and store the appropriate data necessary

    int question_index = -1;
    for (int i = 0; path[i] != '\0'; i++) {
        if (path[i] == '?') {
            question_index = i;
            break;
        }
    }

    if (question_index == -1) {
        // Here we don't have the query string and we exit the program then
        return (0);
    }

    // TODO: Need to make a copy of the string because strtok modifies the original string
    char *query_string = path + question_index + 1;

    char *query_copy = malloc(strlen(query_string) + 1);
    strcpy(query_copy, query_string);

    char *param = strtok(query_copy, "&");

    while(param != NULL) {

        printf("Found a parameter %s\n", param);

        // TODO: Now we need to split those further down and check if they are city or lon and lat.

        char *equals_pos = strchr(param, '=');
        if (equals_pos != NULL) {
            *equals_pos = '\0';
            char *key = param;
            char *value = equals_pos + 1;
           
            // DEBUG INFO
            printf("Key: %s | Value (before decoding hex characters): %s\n", key, value);

            // TODO: Need to now implement checking for each key and value
            if (strcmp(key, "city") == 0 || strcmp(key, "name") == 0 || strcmp(key, "location") == 0) {
                // We store that as the city
                char decoded_city[256];
                url_decoding(decoded_city, value);
                strncpy(params->city, decoded_city, sizeof(params->city) - 1);
                params->city[sizeof(params->city) - 1] = '\0';
                params->has_city = 1;
            } 
            else if (strcmp(key, "lat") == 0 || strcmp(key, "latitude") == 0) {
                strncpy(params->latitude, value, sizeof(params->latitude) - 1);
                params->latitude[sizeof(params->latitude) - 1] = '\0';

            }
            else if (strcmp(key, "lon") == 0 || strcmp(key, "lng") == 0 || strcmp(key, "longitude") == 0) {
                strncpy(params->longitude, value, sizeof(params->longitude) - 1);
                params->longitude[sizeof(params->longitude) - 1] = '\0';
            }
            else {
                // Unknown something
                printf("Unknown parameter: %s=%s\n", key, value);
            }
        }

        param = strtok(NULL, "&");
    }

    if (strlen(params->longitude) > 0 && strlen(params->latitude) > 0) {
        params->has_coordinates = 1;
    }

    free(query_copy);
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

int url_decoding(char *dst, const char *src) {

    char *p = dst;
    while(*src) {
        int hex_value;
        if (*src == '%' && src[1] && src[2]) {
            int hex_value;
            if(sscanf(src + 1, "%2x", &hex_value) == 1) {
                *p++=(char)hex_value;
                src+=3;
            } else {
                *p = *src;
                p++, src++;
            }
        } else if (*src == '+') {
            *p = ' ';
            src++;
        } else {
            *p = *src;
            p++, src++;
        }
    }

    *p = '\0';

    return (0);
}
