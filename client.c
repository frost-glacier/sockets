#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/utsname.h>
#include <arpa/inet.h>
#include <signal.h>

#define PORT 8080

int sock = -1;  // Global socket variable to close it on signal

void close_connection(int sig) {
    if (sock != -1) {
        printf("\nClosing connection...\n");
        close(sock);
    }
    exit(0);
}

int main() {
    struct sockaddr_in server_address;
    char message[1024];
    struct utsname system_info;
    char hostname[256];
    char *username;

    // Set up signal handler to close the connection properly
    signal(SIGINT, close_connection);  // Catch Ctrl+C to gracefully close the connection


    // Create socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Socket creation failed");
        return 1;
    }

    // Configure server address
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);

    // Convert IP address
    if (inet_pton(AF_INET, "127.0.0.1", &server_address.sin_addr) <= 0) {
        perror("Invalid address or address not supported");
        return 1;
    }

    // Connect to the server
    if (connect(sock, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        perror("Connection to server failed");
        return 1;
    }

    printf("Connected to server\n");


    // Keep sending client information to the server in a loop
    while (1) {



        sleep(5);  // keep connection open until process termination
    }

    // Close the socket when the process ends
    close_connection(0);
    return 0;
}
