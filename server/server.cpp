#include <stdio.h>
#include <string.h>
#include "winsock2.h"

#define SERVER_PORT  11393
#define MAX_LINE      256
#define MAX_USERS     100

typedef struct {
    char username[50];
    char password[50];
} User;

User users[MAX_USERS];
int num_users = 0;

void load_users() {
    FILE *file = fopen("D:\\Project\\server\\users.txt", "r");
    if (file == NULL) {
        printf("Error opening users.txt\n");
        return;
    }

    char line[100], temp_user[50], temp_pass[50];
    while (fgets(line, sizeof(line), file)) {
        if (sscanf(line, " (%49[^,], %49[^)])", temp_user, temp_pass) == 2) {
            strcpy(users[num_users].username, temp_user);
            strcpy(users[num_users].password, temp_pass);
            num_users++;
            if (num_users >= MAX_USERS) break;
        }
    }
    fclose(file);
}

int verify_user(const char* username, const char* password) {
    for (int i = 0; i < num_users; i++) {
        if (strcmp(users[i].username, username) == 0 && strcmp(users[i].password, password) == 0) {
            return 1; // User verified
        }
    }
    return 0; // User not found or incorrect password
}

void main() {
    load_users(); // Load users from the file at the start

    // Initialize Winsock.
    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != NO_ERROR) {
        printf("Error at WSAStartup()\n");
        return;
    }

    // Create a socket.
    SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listenSocket == INVALID_SOCKET) {
        printf("Error at socket(): %ld\n", WSAGetLastError());
        WSACleanup();
        return;
    }

    // Bind the socket.
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY; // use local address
    addr.sin_port = htons(SERVER_PORT);
    if (bind(listenSocket, (SOCKADDR*)&addr, sizeof(addr)) == SOCKET_ERROR) {
        printf("bind() failed.\n");
        closesocket(listenSocket);
        WSACleanup();
        return;
    }

    // Listen on the socket.
    if (listen(listenSocket, 5) == SOCKET_ERROR) {
        printf("Error listening on socket.\n");
        closesocket(listenSocket);
        WSACleanup();
        return;
    }

    printf("Waiting for a client to connect...\n");

    SOCKET s;
    while (1) {
        s = accept(listenSocket, NULL, NULL);
        if (s == SOCKET_ERROR) {
            printf("accept() error\n");
            closesocket(listenSocket);
            WSACleanup();
            return;
        }

        printf("Client Connected.\n");

        // Receive login command from the client
        char buf[MAX_LINE];
        int len = recv(s, buf, MAX_LINE, 0);
        buf[len] = 0;

        // Parse the command to verify login
        char command[MAX_LINE], userID[MAX_LINE], password[MAX_LINE];
        if (sscanf(buf, "login %s %s", userID, password) == 2) {
            if (verify_user(userID, password)) {
                char* success_msg = "Login successful!";
                send(s, success_msg, strlen(success_msg), 0);
            }
            else {
                char* error_msg = "Invalid UserID or Password.";
                send(s, error_msg, strlen(error_msg), 0);
            }
        }
        else {
            char* error_msg = "Invalid command format.";
            send(s, error_msg, strlen(error_msg), 0);
        }

        closesocket(s);
        printf("Client Closed.\n");
    }

    closesocket(listenSocket);
    WSACleanup();
}
