#include <stdio.h>
#include <string.h>
#include "winsock2.h"

#define SERVER_PORT  11393
#define MAX_LINE      256

void main(int argc, char** argv) {

    if (argc < 2) {
        printf("\nUsage: client serverName\n");
        return;
    }

    // Initialize Winsock.
    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != NO_ERROR) {
        printf("Error at WSAStartup()\n");
        return;
    }

    // Translate the server name or IP address (128.90.54.1) to resolved IP address
    unsigned int ipaddr;
    if (isalpha(argv[1][0])) {   // host address is a name
        hostent* remoteHost = gethostbyname(argv[1]);
        if (remoteHost == NULL) {
            printf("Host not found\n");
            WSACleanup();
            return;
        }
        ipaddr = *((unsigned long*)remoteHost->h_addr);
    }
    else { // e.g. "128.90.54.1"
        ipaddr = inet_addr(argv[1]);
    }

    // Create a socket.
    SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (s == INVALID_SOCKET) {
        printf("Error at socket(): %ld\n", WSAGetLastError());
        WSACleanup();
        return;
    }

    // Connect to a server.
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = ipaddr;
    addr.sin_port = htons(SERVER_PORT);
    if (connect(s, (SOCKADDR*)&addr, sizeof(addr)) == SOCKET_ERROR) {
        printf("Failed to connect.\n");
        WSACleanup();
        return;
    }

    // Send and receive data.
    char buf[MAX_LINE];
    printf("Enter your command (e.g., login UserID Password): ");
    fgets(buf, sizeof(buf), stdin);

    // Ensure the command is valid (check if it starts with "login")
    char command[MAX_LINE], userID[MAX_LINE], password[MAX_LINE];
    if (sscanf(buf, "login %s %s", userID, password) == 2) {
        // Send the login command to the server
        send(s, buf, strlen(buf), 0);

        // Receive the server's response
        int len = recv(s, buf, MAX_LINE, 0);
        buf[len] = 0;
        printf("Server says: %s\n", buf);
    }
    else {
        printf("Invalid command. Usage: login UserID Password\n");
    }

    closesocket(s);
}
