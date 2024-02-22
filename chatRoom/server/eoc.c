#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 8080
#define SERVER_IP "172.16.104.91"

void error(char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

int main() {
    int client_fd;
    struct sockaddr_in server_addr;

    // Create socket
    if ((client_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        error("Socket creation failed");

    // Set up server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
    server_addr.sin_port = htons(PORT);

    // Connect to the server
    if (connect(client_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1)
        error("Connection to server failed");

    printf("Connected to server on socket %d\n", client_fd);

    while (1) {
        // Read user input
        char message[1024];
        printf("Enter message: ");
        fgets(message, sizeof(message), stdin);

        // Send the message to the server
        send(client_fd, message, strlen(message), 0);
    }

    // Close the client socket
    close(client_fd);

    return 0;
}
