#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include "crypto_utils.h"

#define PORT 8080
#define BUFFER_SIZE 8192

// Thread function declaration
void* receive_messages(void* arg);

void fgets_and_send(int sock, char* buf, size_t sz) {
    if (!fgets(buf, sz, stdin)) return;
    buf[strcspn(buf, "\n")] = '\0';
    send(sock, buf, strlen(buf), 0);
}



int main() {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char message[BUFFER_SIZE] = {0};
    pthread_t receive_thread;

    char *public_key_pem = generate_rsa_pubkey_pem();
    if (!public_key_pem) {
        fprintf(stderr, "Failed to generate RSA keypair\n");
        return 1;
    }

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        perror("inet_pton failed");
        exit(EXIT_FAILURE);
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("connect failed");
        exit(EXIT_FAILURE);
    }

    read(sock, message, BUFFER_SIZE);
    printf("%s", message);

    // Mode L/C
    fgets_and_send(sock, message, BUFFER_SIZE);

    read(sock, message, BUFFER_SIZE);
    printf("%s", message);

    // Username
    fgets_and_send(sock, message, BUFFER_SIZE);

    read(sock, message, BUFFER_SIZE);
    printf("%s", message);

    // Password
    fgets_and_send(sock, message, BUFFER_SIZE);

    read(sock, message, BUFFER_SIZE);
    printf("%s", message);


    if (strstr(message, "Login successful") || strstr(message, "Account created")) {
        for (int i = 0; public_key_pem[i]; i++) {
            if (public_key_pem[i] == '\n' || public_key_pem[i] == '\r') {
                public_key_pem[i] = ' ';
            }
        }
        char pubkey_msg[BUFFER_SIZE];
        snprintf(pubkey_msg, BUFFER_SIZE, "PUBKEY %s\n", public_key_pem);
        send(sock, pubkey_msg, strlen(pubkey_msg), 0);
        printf("Sent public key to server.\n");
    }

    printf("Connected to server!\n");

    pthread_create(&receive_thread, NULL, receive_messages, &sock);

    char current_target[BUFFER_SIZE] = {0};
    char target_pubkey_pem[BUFFER_SIZE] = {0};

    while (1) {
        printf("---> ");
        fflush(stdout);

        if (!fgets(message, BUFFER_SIZE, stdin)) break;

        message[strcspn(message, "\n")] = 0;

        if (strcmp(message, "help") == 0) {
            printf("Available commands:\n");
            printf("1. help - Show this help message\n");
            printf("2. exit - Disconnect from server\n");
            printf("3. /mp username - Start private chat \n");
            continue;
        }

        if (strcmp(message, "exit") == 0) {
            printf("Disconnecting from server...\n");
            break;
        }

        if (strncmp(message, "/mp ", 4) == 0) {
            char target[BUFFER_SIZE];
            sscanf(message + 4, "%s", target);

            if (strlen(target) == 0) {
                printf("Usage: /mp username\n");
                continue;
            }

            send(sock, message, strlen(message), 0);

            current_target[0] = 0;
            target_pubkey_pem[0] = 0;

            continue;
        }

        if (strlen(current_target) > 0 && strlen(target_pubkey_pem) > 0) {
            char *encrypted_b64 = encrypt_with_pubkey(target_pubkey_pem, message);
            if (!encrypted_b64) {
                fprintf(stderr, "Encryption failed\n");
                continue;
            }
            send(sock, encrypted_b64, strlen(encrypted_b64), 0);
            send(sock, "\n", 1, 0);

            free(encrypted_b64);
            continue;
        }

        send(sock, message, strlen(message), 0);
        send(sock, "\n", 1, 0);
    }

    close(sock);
    free(public_key_pem);
    return 0;
}

void* receive_messages(void* arg) {
    int sock = *(int*)arg;
    char buffer[BUFFER_SIZE] = {0};

    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        int bytes_received = read(sock, buffer, BUFFER_SIZE - 1);

        if (bytes_received < 0) {
            perror("read failed");
            break;
        }

        if (bytes_received == 0) {
            printf("Server disconnected\n");
            break;
        }

        buffer[bytes_received] = '\0';

        printf("\n%s\n---> ", buffer);
        fflush(stdout);
    }

    return NULL;
}
