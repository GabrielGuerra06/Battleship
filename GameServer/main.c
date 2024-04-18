#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8888
#define BUFFER_SIZE 1024

int main() {
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    char buffer[BUFFER_SIZE];

    // Create socket
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

    // Initialize server address struct
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(PORT);

    // Bind socket
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("Error binding socket");
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(server_socket, 2) == -1) {
        perror("Error listening");
        exit(EXIT_FAILURE);
    }
    printf("Server listening on port %d...\n", PORT);

    // Accept two clients
    for (int i = 0; i < 2; ++i) {
        if ((client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_addr_len)) == -1) {
            perror("Error accepting connection");
            exit(EXIT_FAILURE);
        }

        printf("Client %d connected.\n", i + 1);

        // Receive and send messages
        while (1) {
            memset(buffer, 0, BUFFER_SIZE);
            if (recv(client_socket, buffer, BUFFER_SIZE, 0) == -1) {
                perror("Error receiving message");
                exit(EXIT_FAILURE);
            }
            printf("Received from client %d: %s\n", i + 1, buffer);

            printf("Enter message to send to client %d: ", (i == 0) ? 2 : 1);
            fgets(buffer, BUFFER_SIZE, stdin);

            if (send(client_socket, buffer, strlen(buffer), 0) == -1) {
                perror("Error sending message");
                exit(EXIT_FAILURE);
            }
        }
    }

    close(server_socket);
    return 0;
}
