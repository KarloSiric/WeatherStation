/*
* @Author: karlosiric
* @Date:   2025-06-26 14:39:26
* @Last Modified by:   karlosiric
* @Last Modified time: 2025-06-27 20:43:29
*/

#include "../include/http_server.h"

int start_http_server(void) {

    int socket_fd, bind_result;
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd == -1) {
        printf("Error creating socket: %s\n", strerror(errno));
        return (-1);
    }

    struct sockaddr_in server_address;
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




    return (0);
}
