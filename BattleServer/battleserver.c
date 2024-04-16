#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

// --- Bool Values ---
#define TRUE                1
#define FALSE               0
// --- Errors & Actions ---
// --- Error Codes ---
#define SUCCESS             0
#define FAILURE             (-1)
#define CREATE_DATABASE     (-2)
// --- Action Codes ---
#define LOGIN               0b00001000 // 8
#define REGISTER            0b00001001 // 9
// --- Server Configuration ---
#define DB                  "users.txt"
#define BUFFER_SIZE         1024
#define PORT                5000
#define LOBBY_SIZE          2
#define MAX_PLAYERS         256
#define MAX_USERNAME_LEN    20
#define AUTH_KEY_LEN        4

struct Player {
    char username[MAX_USERNAME_LEN + 1];
    char auth_key[AUTH_KEY_LEN];
    uint8_t turn;
    uint8_t lobby;
};

struct PlayerJoin {
    char action;
    char username[MAX_USERNAME_LEN + 1];
    char auth_key[AUTH_KEY_LEN + 1];
};

struct Player players[MAX_PLAYERS];

int check(int expression, const char *msg_error);

void create_database();

int load_players();

void register_user(const char *username, const char *auth_key);

int validate_credentials(const char *username, const char *auth_key);

void handle_client(int client_socket);

void checkEntryAction(char action, const char *username, const char *auth_key);

void print_players(int num_players);


int main() {
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    pid_t child_pid;
    int opt = 1; // Option value for setsockopt

    check(server_socket = socket(AF_INET, SOCK_STREAM, 0), "Socket error");

    check(setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)), "Set socket option");

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    check(bind(server_socket, (struct sockaddr *) &server_addr, sizeof(server_addr)), "Bind error");
    check(listen(server_socket, LOBBY_SIZE), "Listen error");
    printf("Server listening on port %d\n", PORT);
    check(load_players(), "");
    printf("Papu loaded into server.\n");
    while (TRUE) {
        client_socket = accept(server_socket, (struct sockaddr *) &client_addr, &client_addr_len);
        if (client_socket < 0) {
            perror("Accept");
            continue;
        }

        printf("New client connected\n");

        child_pid = fork();
        if (child_pid == 0) {
            close(server_socket);
            handle_client(client_socket);
            exit(SUCCESS);
        } else if (child_pid < 0) {
            perror("Fork");
            exit(FAILURE);
        } else {
            close(client_socket);
        }
    }

    close(server_socket);

    return 0;
}

int check(int expression, const char *msg_error) {
    switch (expression) {
        case -1:
            perror(msg_error);
            exit(EXIT_FAILURE);
        case CREATE_DATABASE:
            create_database();
            return SUCCESS;
        default:
            break;
    }

    return expression;
}

void create_database() {
    FILE *db = fopen(DB, "w+");
    if (db == NULL) {
        perror("Error creating Database");
        return;
    }
    fclose(db);
    printf("Database created\n");
}

int load_players() {
    FILE *db = fopen(DB, "r");
    if (db != NULL) {
        char *buffer = malloc(sizeof(char) * (MAX_USERNAME_LEN + AUTH_KEY_LEN + 2)); // +2 for newline and null terminator
        if (buffer != NULL) {
            int user_index = 0;
            while (fgets(buffer, MAX_USERNAME_LEN + AUTH_KEY_LEN + 2, db)) { // pass correct buffer size
                char *token = strtok(buffer, " ");
                if (token != NULL) {
                    strcpy(players[user_index].username, token);
                    token = strtok(NULL, " ");
                    if (token != NULL) {
                        strcpy(players[user_index].auth_key, token);
                        players[user_index].lobby = 0;
                        players[user_index].turn = 0;
                        user_index++;
                    }
                }
            }
            free(buffer);
            print_players(user_index); // Print the loaded players

        } else {
            perror("Memory allocation failed");
            exit(FAILURE);
        }
    } else {
        printf("Database doesn't exist. Creating a new one\n");
        return CREATE_DATABASE;
    }
    fclose(db);
    printf("Users loaded into server.\n");

    return SUCCESS;
}

#define MAX_BUFFER_SIZE 30 // Maximum size of the buffer to store client info


void handle_client(int client_socket) {
    char buffer[MAX_BUFFER_SIZE];
    const char *ask_username = "[register/login][username][auth_key]:\n";
    send(client_socket, ask_username, strlen(ask_username), 0);
    int bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0);
    if (bytes_received < 0) {
        perror("Error reading from socket");
        exit(EXIT_FAILURE);
    }


    // Add null terminator to make it a valid C string
    buffer[bytes_received] = '\0';
    printf("Received message from client: %s\n", buffer);
    //select buffer[1] to buffer[24] and make it username
    char username[MAX_USERNAME_LEN + 1];
    strncpy(username, buffer + 1, MAX_USERNAME_LEN);
    username[MAX_USERNAME_LEN] = '\0';

    //select buffer[25] to buffer[28] and make it auth
    char auth[AUTH_KEY_LEN + 1];
    strncpy(auth, buffer + 25, AUTH_KEY_LEN);
    auth[AUTH_KEY_LEN] = '\0';

    checkEntryAction(buffer[0], username, auth);
}



int validate_credentials(const char *username, const char *auth_key) {
//    FILE *user_file = fopen(DB, "r");
//    char line[BUFFER_SIZE];
//    char file_username[MAX_USERNAME_LEN + 1];
//    char file_auth_key[AUTH_KEY_LEN + 1];
//
//    if (user_file == NULL) {
//        perror("Error opening user file");
//        return 0;
//    }
//
//    while (fgets(line, BUFFER_SIZE, user_file) != NULL) {
//        sscanf(line, "%s %s", file_username, file_auth_key);
//        if (strcmp(username, file_username) == 0 && strcmp(auth_key, file_auth_key) == 0) {
//            fclose(user_file);
//            return 1;
//        }
//    }
//
//    fclose(user_file);
//    return 0;
    for (int i = 0; i < MAX_PLAYERS; ++i) {
        if (strcmp(username, players[i].username) == 0 && strcmp(auth_key, players[i].auth_key) == 0) {
            return TRUE;
        }
    }

    return FALSE;
}


void checkEntryAction(char action, const char *username, const char *auth_key) {
    printf("Users loaded into server.\n");

    if (action == LOGIN) {
        printf("Iniciar sesión del usuario");
//     Insertar accion de login de usuario
    } else if (action == REGISTER) {
        printf("Registro de usuario");
        register_user(username, auth_key);

    } else {
        printf("Accion incorrecta");
    }
}

void register_user(const char *username, const char *auth_key) {
    FILE *user_file = fopen(DB, "a");
    if (user_file == NULL) {
        perror("Error opening user file");
        return;
    }
    fprintf(user_file, "%s %s\n", username, auth_key);

    fclose(user_file);

}

void print_players(int num_players) {
    printf("Players loaded into server:\n");
    for (int i = 0; i < num_players; i++) {
        printf("Username: %s, Auth Key: %s\n", players[i].username, players[i].auth_key);
    }
}

//void temp(int client_socket) {
//    char buffer[BUFFER_SIZE];
//    int bytes_received;
//    char username[MAX_USERNAME_LEN + 1];
//    char auth_key[AUTH_KEY_LEN + 1];
//    bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0);
//    const char *prompt = "Enter 'register' to register or 'login' to login: ";
//    send(client_socket, prompt, strlen(prompt), 0);
//// [Action][Name][Password]
//// 00001000 00000000 x 24 00000000 x 4 -> Login
//// 00001001 00000000 x 24 00000000 x 4 -> Register
//    bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0);
//    buffer[bytes_received] = '\0';
//    printf("Received: %s\n", buffer);
//
//    if (strcmp(buffer, "register") == 0) {
////        register_user(client_socket);
//    } else if (strcmp(buffer, "login") == 0) {
//        bytes_received = recv(client_socket, username, MAX_USERNAME_LEN, 0);
//        username[bytes_received] = '\0';
//        bytes_received = recv(client_socket, auth_key, AUTH_KEY_LEN, 0);
//        auth_key[bytes_received] = '\0';
//
//        if (validate_credentials(username, auth_key)) {
//            printf("Client '%s' authenticated successfully\n", username);
//            // Proceed with the game
//            // ...
//        } else {
//            printf("Client '%s' authentication failed\n", username);
//            const char *auth_failed = "Authentication failed. Connection closed.\n";
//            send(client_socket, auth_failed, strlen(auth_failed), 0);
//            close(client_socket);
//        }
//    } else {
//        printf("Client entered invalid choice: %s\n", buffer);
//        const char *invalid_choice = "Invalid choice. Connection closed.\n";
//        send(client_socket, invalid_choice, strlen(invalid_choice), 0);
//        close(client_socket);
//    }
//}