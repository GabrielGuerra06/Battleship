#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define PORT 8888
#define MAX_CLIENTS 2
#define BUFFER_SIZE 1024

int main() {
    int server_fd, client_sockets[MAX_CLIENTS];
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len;
    char buffer[BUFFER_SIZE];
    int client_count = 0;
    int opt = 1;

    // Crear el socket del servidor
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        printf("Error al crear el socket\n");
        return 1;
    }

    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // Configurar la dirección del servidor
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Vincular el socket a la dirección del servidor
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        printf("Error al vincular el socket\n");
        return 1;
    }

    // Escuchar conexiones entrantes
    if (listen(server_fd, MAX_CLIENTS) == -1) {
        printf("Error al escuchar conexiones\n");
        return 1;
    }

    printf("Servidor iniciado. Esperando conexiones...\n");

    while (client_count < MAX_CLIENTS) {
        client_len = sizeof(client_addr);
        client_sockets[client_count] = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
        if (client_sockets[client_count] == -1) {
            printf("Error al aceptar conexión\n");
            return 1;
        }

        printf("Cliente %d conectado\n", client_count + 1);
        client_count++;
    }

    printf("Dos clientes conectados. Enviando mensaje...\n");

    char position[BUFFER_SIZE];
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (i == 0) {
            // Prompt the first player to select a position
            //clean
            strcpy(buffer, "Prueba posicion:");
            send(client_sockets[i], buffer, strlen(buffer), 0);
            memset(buffer, 0, sizeof(buffer));
            ssize_t recv_len = recv(client_sockets[i], buffer, BUFFER_SIZE, 0);
            buffer[recv_len] = '\0';
            strcpy(position, buffer);
            printf("Posicion recibida del jugador %d: %s\n", i + 1, position);
        }
        else {
            strcpy(buffer, "Espera tu turno.");
            send(client_sockets[i], buffer, strlen(buffer), 0);
            printf("El jugador %d está esperando su turno.\n", i + 1);
        }
    }

    int send_cl = send(client_sockets[1], position, strlen(position), 0);
    if (send_cl == -1) {
        perror("Send Error");
        printf("LN 80 In: %d\n", send_cl);
    }
    printf("LN 80 Out: %d\n", send_cl);


    memset(buffer, 0, sizeof(buffer));
    ssize_t recv_len = recv(client_sockets[1], buffer, BUFFER_SIZE, 0);
    printf("LN 90: %zd\n", recv_len);
    buffer[recv_len] = '\0';
    //Handle cases of the buffer answer
    if (strcmp(buffer, "ATINASTE") == 0) {
        strcpy(buffer, "ATINASTE");
    }
    else if (strcmp(buffer, "FALLASTE") == 0) {
        printf("oops!\n");
    }
    else if (strcmp(buffer, "DERROTA") == 0) {
        printf("Perdiste!\n");
    }

    // Cerrar los sockets de los clientes y el socket del servidor
    for (int i = 0; i < MAX_CLIENTS; i++) {
        close(client_sockets[i]);
    }
    close(server_fd);

    return 0;
}