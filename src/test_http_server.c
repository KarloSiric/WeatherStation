/*
* @Author: karlosiric
* @Date:   2025-06-27 21:40:04
* @Last Modified by:   karlosiric
* @Last Modified time: 2025-07-08 12:29:30
*/

#include "../include/http_server.h"
#include <signal.h>

volatile int keep_running = 1;

void signal_handler(int sig) {
    printf("\nShutting down server...\n");
    keep_running = 0;
}

int main(void) {
    printf("Starting HTTP Server Test...\n");
    
    // Set up signal handler for Ctrl+C
    signal(SIGINT, signal_handler);
    
    // Start your HTTP server function
    int result = start_http_server();
    
    if (result != 0) {
        fprintf(stderr, "HTTP Server failed to start\n");
        return 1;
    }
    
    printf("Server stopped.\n");
    return 0;

}
