/*
* @Author: karlosiric
* @Date:   2025-06-27 21:40:04
* @Last Modified by:   karlosiric
* @Last Modified time: 2025-07-14 14:45:28
*/

#include "../include/http_server.h"
#include "../include/logger.h"

int main(void) {
    printf("Starting HTTP Server Test...\n");

    if (logger_init("logs/server.log") != 0) {
        printf("Warning: Could not initialize the logger\n");
    }
    else {
        printf("Logger initialized successfully!\n");
    }
    
    
    // Start your HTTP server function
    int result = start_http_server();
    
    if (result != 0) {
        fprintf(stderr, "HTTP Server failed to start\n");
        return 1;
    }
    
    printf("Server stopped.\n");
    return 0;

}
