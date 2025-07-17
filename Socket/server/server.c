#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define PORT 8080
#define BUFFER_SIZE 1024
#define MAX_CLIENTS 10

int main() {
    int server_fd, client_fd;
    struct sockaddr_in address;
    char buffer[BUFFER_SIZE] = {0};
    int client_sockets[MAX_CLIENTS] = {0};  // Array to store client sockets
    
    // Create socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    
    // Bind socket to port
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    
    // Listen for connections
    if (listen(server_fd, 3) < 0) {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }
    
    printf("Server listening on port %d \n", PORT);
    
    while(1) {
        // Accept new connection
        client_fd = accept(server_fd, NULL, NULL);
        if (client_fd < 0) {
            perror("accept failed");
            continue;
        }
        
        // Find first available slot in client_sockets array
        int i;
        for (i = 0; i < MAX_CLIENTS; i++) {
            if (client_sockets[i] == 0) {
                client_sockets[i] = client_fd;
                break;
            }
        }
        
        if (i == MAX_CLIENTS) {
            printf("Maximum clients reached\n");
            close(client_fd);
            continue;
        }
        
        // Handle messages from this client
        while(1) {
            memset(buffer, 0, BUFFER_SIZE);
            
            int bytes_received = read(client_fd, buffer, BUFFER_SIZE);
            if (bytes_received < 0) {
                perror("read failed");
                break;
            }
            
            if (bytes_received == 0) {
                printf("Client disconnected\n");
                break;
            }
            
            // Broadcast message to all connected clients
            for (int j = 0; j < MAX_CLIENTS; j++) {
                if (client_sockets[j] != 0 && client_sockets[j] != client_fd) {
                    send(client_sockets[j], buffer, bytes_received, 0);
                }
            }
        }
        
        // Remove disconnected client from array
        client_sockets[i] = 0;
        close(client_fd);
    }
    
    return 0;
}