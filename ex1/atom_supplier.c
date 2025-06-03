#include <poll.h>
#include <unistd.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <arpa/inet.h>

#define EXIT_FAILURE 1
#define NFDS 2

int main(int argc, char *argv[])
{
    int running = 1;
    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s <server_ip> <port>\n", argv[0]);
        return 1;
    }

    const char *server_ip = argv[1];
    int port = atoi(argv[2]);

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        perror("socket");
        return 1;
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0)
    {
        perror("inet_pton");
        close(sockfd);
        return 1;
    }

    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("connect");
        close(sockfd);
        return 1;
    }

    // //receive a message from the server
    struct pollfd fds[2];
    fds[0].fd = sockfd;
    fds[0].events = POLLIN;
    fds[1].fd = STDIN_FILENO;
    fds[1].events = POLLIN;

    printf("Connected to warehouse server at %s:%d\n", server_ip, port);
    while (running)
    {
        int poll_count = poll(fds, NFDS, -1);
        if (poll_count < 0)
        {
            perror("poll");
            break;
        }
        if (fds[0].revents & POLLIN)
        {
            int code;
            ssize_t bytes_read = read(sockfd, &code, sizeof(code));
            if (bytes_read <= 0)
            {
                perror("read");
                break;
            }
          
        }
        if (fds[1].revents & POLLIN)
        {
            char buffer[1024];
            fgets(buffer, sizeof(buffer), stdin);

            buffer[strcspn(buffer, "\n")] = 0;

            if (strcmp(buffer, "EXIT") == 0)
            {
                printf("Exiting by user request.\n");
                running = 0;
                close(sockfd);
                break;
            }

            send(sockfd, buffer, sizeof(buffer), 0);
            printf("Sent message to server: %s\n", buffer);
        }
    }
}