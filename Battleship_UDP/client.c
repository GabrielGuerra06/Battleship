#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/select.h>

#define SERVER_IP "172.30.243.244"
#define PORT 8008
#define BUFFER_SIZE 1024

int main() {
    int client_fd;
    struct sockaddr_in server_addr;
    socklen_t addr_len = sizeof(struct sockaddr_in);
    char buffer[BUFFER_SIZE];
    // Create UDP socket
    client_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (client_fd < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Initialize server address structure
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
    server_addr.sin_port = htons(PORT);

    char byte[1] = {1};
    if (sendto(client_fd, byte, strlen(byte), 0, (struct sockaddr *)&server_addr, addr_len) < 0) {
        perror("Error al enviar mensaje");
        return -1;
    }
    
    // Message chat loop
    while (1) {
        printf("Ingrese un mensaje: ");
        fgets(buffer, BUFFER_SIZE, stdin);
        buffer[strcspn(buffer, "\n")] = '\0'; // Eliminar el salto de línea

        if (sendto(client_fd, buffer, strlen(buffer), 0, (struct sockaddr *)&server_addr, addr_len) < 0) {
            perror("Error al enviar mensaje");
            break;
        }

        memset(buffer, 0, BUFFER_SIZE);
        if (recvfrom(client_fd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&server_addr, &addr_len) < 0) {
            perror("Error al recibir mensaje");
            break;
        }

        printf("Mensaje recibido del servidor: %s\n", buffer);
    }

    close(client_fd);
    return 0;
}
