#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/epoll.h>

#define MAX_CLIENTS 10
#define MAX_EVENTS 10
#define PORT 8080

void error(char *msg)
{
    perror(msg);
    exit(EXIT_FAILURE);
}

int main()
{
    int server_fd, client_fds[MAX_CLIENTS], epoll_fd, events_count;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    struct epoll_event ev, events[MAX_EVENTS];

    // Create socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        error("Socket creation failed");

    // Set up server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Bind the socket
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
        error("Binding failed");

    // Listen for incoming connections
    if (listen(server_fd, MAX_CLIENTS) == -1)
        error("Listening failed");

    // Create epoll instance
    epoll_fd = epoll_create1(0);
    if (epoll_fd == -1)
        error("Epoll creation failed");

    // Add the server socket to the epoll set
    ev.events = EPOLLIN;
    ev.data.fd = server_fd;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &ev) == -1)
        error("Adding server socket to epoll set failed");

    printf("Server listening on port %d...\n", PORT);

    while (1)
    {
        // Wait for events on the epoll set
        events_count = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        if (events_count == -1)
            error("Epoll wait failed");

        for (int i = 0; i < events_count; ++i)
        {
            if (events[i].data.fd == server_fd)
            {
                // Accept new connection
                int new_client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
                if (new_client_fd == -1)
                    error("Accepting new connection failed");

                printf("New connection from %s on socket %d\n", inet_ntoa(client_addr.sin_addr), new_client_fd);

                // Add the new client socket to the epoll set
                ev.events = EPOLLIN;
                ev.data.fd = new_client_fd;
                if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, new_client_fd, &ev) == -1)
                    error("Adding new client socket to epoll set failed");
            }
            else
            {
                char buffer[1024];
                int client_socket = events[i].data.fd;
                int bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);

                if (bytes_received <= 0)
                {
                    // Connection closed or error
                    printf("Client on socket %d disconnected\n", client_socket);
                    close(client_socket);
                    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client_socket, NULL);
                }
                else
                {
                    // Broadcast the message to all clients
                    for (int j = 0; j < MAX_CLIENTS; ++j)
                    {
                        if (client_fds[j] != server_fd && client_fds[j] != client_socket)
                            send(client_fds[j], buffer, bytes_received, 0);
                    }
                }
            }
        }
    }

    // Close the server socket
    close(server_fd);

    return 0;
}
