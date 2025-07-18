#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>

#define PORT 8080
#define BUFFER_SIZE 1024

// Function to handle receiving messages from server
void* receive_messages(void* arg) {
    int sock = *(int*)arg;
    char buffer[BUFFER_SIZE] = {0};
    
    while(1) {
        memset(buffer, 0, BUFFER_SIZE);
        int bytes_received = read(sock, buffer, BUFFER_SIZE);
        
        if (bytes_received < 0) {
            perror("read failed");
            break;
        }
        
        if (bytes_received == 0) {
            printf("Server disconnected\n");
            break; 
        }
        
        printf("%s\n", buffer);
    }
    
    return NULL;
}

int main() {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char message[BUFFER_SIZE] = {0};
    pthread_t receive_thread;
    
    // Create socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    
    // Convert IP address from string to binary format
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        perror("inet_pton failed");
        exit(EXIT_FAILURE);
    }
    
    // Connect to server
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("connect failed");
        exit(EXIT_FAILURE);
    }
        // Read prompt from server
    read(sock, message, BUFFER_SIZE);
    printf("%s", message);  // Should print: Enter your pseudo:

    // User enters pseudo
    fgets(message, BUFFER_SIZE, stdin);
    send(sock, message, strlen(message), 0);

        
    printf("Connected to server!\n");
    
    // Create thread to receive messages
    pthread_create(&receive_thread, NULL, receive_messages, &sock);
    
    while(1) {
        // Get message from user
        printf("---> ");
        fgets(message, BUFFER_SIZE, stdin);

        // Remove newline from input
        message[strcspn(message, "\n")] = 0;
     
        // Check for exit command
        if (strncmp(message, "help", 4) == 0) {
            printf("Available commands:\n");
            printf("1. help - Show this help message\n");
            printf("2. exit - Disconnect from server\n");
            printf("3. mp - send mp to any person\n"); //TODO - add a mp feature
            continue;
        }
          
        if (strlen(message) == 0) {
            continue; // Skip empty messages
        }
        
        if (strncmp(message, "/exit", 4) == 0) {
            break;
        }
        
        // Send message to server
        send(sock, message, strlen(message), 0);
    }
    
    // Close socket
    close(sock);
    return 0;
}
