/*
* @Author: karlosiric
* @Date:   2025-06-26 14:39:26
* @Last Modified by:   karlosiric
* @Last Modified time: 2025-06-27 19:16:34
*/

#include "../include/http_server.h"

int main(void) {

    int socket_fd = socket(PF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) {
        perror("Failed to create a socket!");
        return (-1);
    }

    printf("Successfully created a socket with file descriptor: %d\n", socket_fd);




    return (0);
}
