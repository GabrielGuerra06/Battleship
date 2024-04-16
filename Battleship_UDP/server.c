#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

// --- Bool Values ---
#define TRUE                1
#define FALSE               0
// --- Error Codes ---
#define SUCCESS             0
#define FAILURE             (-1)
// --- Server Configuration ---
#define PORT 8008
#define BUFFER_SIZE 1024

struct Client {
    ssize_t client_fd;
    struct sockaddr_in client_addr;
    socklen_t addr_len;
};

int check(int expression, const char *msg_error);
int check_transfer(ssize_t expression, const char *msg_error);


int main() {
    int server_fd;
    struct sockaddr_in server_addr;

    server_fd = check(socket(AF_INET, SOCK_DGRAM, 0), "Socket Error");

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    check(bind(server_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)), "Bind Error");

    printf("UDP Messaging Server listening on port %d...\n", PORT);

    struct Client client_1, client_2;
    client_1.addr_len = sizeof(client_1.client_addr);
    client_2.addr_len = sizeof(client_2.client_addr);


    while (TRUE) {
        char buffer_c1[BUFFER_SIZE];
        char buffer_c2[BUFFER_SIZE];
//        ssize_t client_1.client_fd, client_2.client_fd;
        //Receive from client 1
        client_1.client_fd = recvfrom(server_fd, buffer_c1, BUFFER_SIZE, MSG_DONTWAIT, (struct sockaddr *) &client_1.client_addr, &client_1.addr_len);
        //Receive from client 2
        client_2.client_fd = recvfrom(server_fd, buffer_c2, BUFFER_SIZE, MSG_DONTWAIT, (struct sockaddr *) &client_2.client_addr, &client_2.addr_len);
        if(client_1.client_fd == FAILURE && client_2.client_fd == FAILURE) {
//            perror("Receive From Error");
            continue;
        } else if (client_1.client_fd != FAILURE && client_2.client_fd == FAILURE) {
            buffer_c1[client_1.client_fd] = '\0';
            printf("Client %s:%d: %s, %zd\n", inet_ntoa(client_1.client_addr.sin_addr), ntohs(client_1.client_addr.sin_port), buffer_c1, client_1.client_fd);
            // Send to client 2
            check_transfer(sendto(server_fd, buffer_c2, strlen(buffer_c2), 0, (struct sockaddr *) &client_1.client_addr, client_1.addr_len), "Send To Error");
        } else if (client_1.client_fd == FAILURE && client_2.client_fd != FAILURE) {
            buffer_c2[client_2.client_fd] = '\0';
            printf("Client %s:%d: %s, %zd\n", inet_ntoa(client_2.client_addr.sin_addr), ntohs(client_2.client_addr.sin_port), buffer_c2, client_2.client_fd);
            //Sent to client 1
            check_transfer(sendto(server_fd, buffer_c1, strlen(buffer_c1), 0, (struct sockaddr *) &client_2.client_addr, client_2.addr_len), "Send To Error");
        } else {
            buffer_c1[client_1.client_fd] = '\0';
            buffer_c2[client_2.client_fd] = '\0';
            printf("Client %s:%d: %s, %zd\n", inet_ntoa(client_1.client_addr.sin_addr), ntohs(client_1.client_addr.sin_port), buffer_c1, client_1.client_fd);
            printf("Client %s:%d: %s, %zd\n", inet_ntoa(client_2.client_addr.sin_addr), ntohs(client_2.client_addr.sin_port), buffer_c2, client_2.client_fd);
            // Send to client 2
            check_transfer(sendto(server_fd, buffer_c1, strlen(buffer_c1), 0, (struct sockaddr *) &client_2.client_addr, client_2.addr_len), "Send To Error");
            //Sent to client 1
            check_transfer(sendto(server_fd, buffer_c2, strlen(buffer_c2), 0, (struct sockaddr *) &client_1.client_addr, client_1.addr_len), "Send To Error");
        }
    }

    // Cerrar socket
    close(server_fd);

    return 0;
}

int check(int expression, const char *msg_error) {
    if (expression == FAILURE) {
        perror(msg_error);
        exit(FAILURE);
    }

    return expression;
}

int check_transfer(ssize_t expression, const char *msg_error) {
    if (expression == FAILURE) {
        perror(msg_error);
        return FAILURE;
    }

    return expression;
}