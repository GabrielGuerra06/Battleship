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
#define LOG_OUT             "Logging out from Chat - Bye bye :)"
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

void send_package(int server_fd, char *buffer, ssize_t n, struct Client *sender, struct Client *receiver);

struct Client empty();

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

    struct Client client_1 = empty();
    struct Client client_2 = empty();
    struct Client empty_client = empty();

    while (TRUE) {
        char buffer[BUFFER_SIZE];
        struct Client client_any;
        client_any.addr_len = sizeof(client_any.client_addr);

        // Receive from any client
        ssize_t data = recvfrom(server_fd, buffer, BUFFER_SIZE, MSG_DONTWAIT,
                                (struct sockaddr *) &client_any.client_addr, &client_any.addr_len);

        buffer[data] = '\0';

        if (data == FAILURE) continue;

        if (strcmp(buffer, LOG_OUT) == 0) {
            if (memcmp(&client_any, &client_1, sizeof(struct Client)) == 0) client_1 = empty();
            else if (memcmp(&client_any, &client_2, sizeof(struct Client)) == 0) client_2 = empty();
            printf("Client %s:%d: Logging Off..., %zd\n", inet_ntoa(client_any.client_addr.sin_addr), ntohs(client_any.client_addr.sin_port), data);
            continue;
        }
        // Player 1 sends to Player 2 if connected
        if (memcmp(&client_any, &client_1, sizeof(struct Client)) == 0)
            send_package(server_fd, buffer, data, &client_1, &client_2);
        // Player 2 sends to Player 1 if connected
        else if (memcmp(&client_any, &client_2, sizeof(struct Client)) == 0)
            send_package(server_fd, buffer, data, &client_2, &client_1);
        // If there is no Player X, take the place to communicate
        else {
            // Take place Player 1
            if (memcmp(&client_1, &empty_client, sizeof(struct Client)) == 0) {
                client_1 = client_any;
                send_package(server_fd, buffer, data, &client_1, &client_2);
            }
            // Take place Player 2
            else if (memcmp(&client_2, &empty_client, sizeof(struct Client)) == 0) {
                client_2 = client_any;
                send_package(server_fd, buffer, data, &client_2, &client_1);
            }
        }
    }

    // Cerrar socket
    close(server_fd);
    return SUCCESS;
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

void send_package(int server_fd, char *buffer, ssize_t n, struct Client *sender, struct Client *receiver) {
    printf("Client %s:%d: %s, %zd\n", inet_ntoa(sender->client_addr.sin_addr),
           ntohs(sender->client_addr.sin_port), buffer, n);
    // Send to sender
    check_transfer(sendto(server_fd, buffer, BUFFER_SIZE, 0, (struct sockaddr *) &sender->client_addr,
                          sender->addr_len), "Send To Error");
    //Sent to receiver
    check_transfer(sendto(server_fd, buffer, BUFFER_SIZE, 0, (struct sockaddr *) &receiver->client_addr,
                          receiver->addr_len), "Send To Error");
}

struct Client empty() {
    struct Client empty;
    memset(&empty, 0, sizeof(struct Client));
    return empty;
}