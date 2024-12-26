#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <time.h>

#define PORT 8080
#define BUFFER_SIZE 1024
#define LOG_FILE "server_logs.txt"

int client_count = 0;
pthread_mutex_t client_count_lock;

void write_log(const char *log_message) {
    FILE *log_file = fopen(LOG_FILE, "a");
    if (log_file == NULL) {
        perror("Failed to open log file");
        return;
    }
    fprintf(log_file, "%s\n", log_message);
    fclose(log_file);
}

void *handle_client(void *socket_desc) {
    int client_socket = *(int *)socket_desc;
    free(socket_desc);

    char buffer[BUFFER_SIZE];
    ssize_t read_size;

    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);
    getpeername(client_socket, (struct sockaddr *)&client_addr, &addr_len);

    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
    int client_port = ntohs(client_addr.sin_port);

    // Get current time
    time_t now = time(NULL);
    struct tm *timeinfo = localtime(&now);
    char time_buffer[64];
    strftime(time_buffer, sizeof(time_buffer), "%Y-%m-%d %H:%M:%S", timeinfo);

    // Log client connection
    char log_message[256];
    snprintf(log_message, sizeof(log_message), "[%s] Client connected: IP %s, Port %d", time_buffer, client_ip, client_port);
    write_log(log_message);

    pthread_mutex_lock(&client_count_lock);
    client_count++;
    printf("Client connected: IP %s, Port %d. Active clients: %d\n", client_ip, client_port, client_count);
    pthread_mutex_unlock(&client_count_lock);

    while ((read_size = recv(client_socket, buffer, BUFFER_SIZE, 0)) > 0) {
        buffer[read_size] = '\0';
        printf("Client (%s:%d) message: %s\n", client_ip, client_port, buffer);
        send(client_socket, "Acknowledged", 13, 0);
    }

    pthread_mutex_lock(&client_count_lock);
    client_count--;
    printf("Client disconnected: IP %s, Port %d. Active clients: %d\n", client_ip, client_port, client_count);
    pthread_mutex_unlock(&client_count_lock);

    // Log client disconnection
    snprintf(log_message, sizeof(log_message), "[%s] Client disconnected: IP %s, Port %d", time_buffer, client_ip, client_port);
    write_log(log_message);

    close(client_socket);
    return NULL;
}

int main() {
    int server_socket, client_socket, *new_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);

    pthread_mutex_init(&client_count_lock, NULL);

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Socket creation failed");
        return 1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        return 1;
    }

    if (listen(server_socket, 10) < 0) {
        perror("Listen failed");
        return 1;
    }

    printf("Server listening on port %d\n", PORT);

    while ((client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &addr_len))) {
        printf("New connection accepted\n");
        new_sock = malloc(sizeof(int));
        *new_sock = client_socket;

        pthread_t client_thread;
        if (pthread_create(&client_thread, NULL, handle_client, (void *)new_sock) < 0) {
            perror("Thread creation failed");
            free(new_sock);
        }

        pthread_detach(client_thread);
    }

    close(server_socket);
    pthread_mutex_destroy(&client_count_lock);
    return 0;
}

