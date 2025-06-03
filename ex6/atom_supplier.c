#include <poll.h>
#include <unistd.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/types.h>

#define EXIT_FAILURE 1
#define NFDS 2

extern char *optarg;

int main(int argc, char *argv[])
{
    char *hostname = NULL;
    char *port_str = NULL;
    char *socket_path = NULL;
    int c;
    int running = 1;


    // Parse command line arguments
    while ((c = getopt(argc, argv, "h:p:f:")) != -1)
    {
        switch (c)
        {
        case 'h':
            hostname = optarg;
            break;
        case 'p':
            port_str = optarg;
            break;
        case 'f':
            socket_path = optarg;
            break;
        case '?':
            return 1;
        }
    }

    // Validate arguments
    int has_inet = (hostname != NULL && port_str != NULL);
    int has_unix = (socket_path != NULL);

    if (!has_inet && !has_unix)
    {
        fprintf(stderr, "Error: Must specify either inet socket (-h -p) or unix socket (-f)\n");
        return 1;
    }

    if (has_inet && has_unix)
    {
        fprintf(stderr, "Error: Cannot use both inet socket and unix socket simultaneously\n");
        return 1;
    }

    int sockfd;
    struct sockaddr_un unix_addr;

    if (has_inet)
    {
        struct addrinfo hints, *result, *rp;

        // Initialize hints
        memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_INET;       // IPv4
        hints.ai_socktype = SOCK_STREAM; // TCP
        hints.ai_flags = 0;
        hints.ai_protocol = 0;

        // Get address info
        int status = getaddrinfo(hostname, port_str, &hints, &result);
        if (status != 0)
        {
            fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
            return 1;
        }

        // Try each address until we successfully connect
        sockfd = -1;
        for (rp = result; rp != NULL; rp = rp->ai_next)
        {
            sockfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
            if (sockfd == -1)
                continue;

            if (connect(sockfd, rp->ai_addr, rp->ai_addrlen) != -1)
                break; // Success

            close(sockfd);
            sockfd = -1;
        }

        freeaddrinfo(result);

        if (sockfd == -1)
        {
            fprintf(stderr, "Could not connect to %s:%s\n", hostname, port_str);
            return 1;
        }

        printf("Connected to server at %s:%s\n", hostname, port_str);
    }
    else
    {
        // Create unix socket
        sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
        if (sockfd < 0)
        {
            perror("socket");
            return 1;
        }

        memset(&unix_addr, 0, sizeof(unix_addr));
        unix_addr.sun_family = AF_UNIX;
        strncpy(unix_addr.sun_path, socket_path, sizeof(unix_addr.sun_path) - 1);

        if (connect(sockfd, (struct sockaddr *)&unix_addr, sizeof(unix_addr)) < 0)
        {
            perror("connect");
            close(sockfd);
            return 1;
        }

        printf("Connected to server via Unix socket: %s\n", socket_path);
    }

    // Setup polling
    struct pollfd fds[2];
    fds[0].fd = sockfd;
    fds[0].events = POLLIN;
    fds[1].fd = STDIN_FILENO;
    fds[1].events = POLLIN;

    printf("Type messages to send to server. Type 'EXIT' to quit.\n");
    printf("> ");
    fflush(stdout);

    while (running)
    {
        int poll_count = poll(fds, NFDS, -1);
        if (poll_count < 0)
        {
            perror("poll");
            break;
        }

        // Check if socket has data (server sent something or closed connection)
        if (fds[0].revents & POLLIN)
        {
            char server_buffer[1024];
            ssize_t bytes_read = read(sockfd, server_buffer, sizeof(server_buffer) - 1);
            if (bytes_read <= 0)
            {
                if (bytes_read == 0)
                {
                    printf("\nServer closed the connection.\n");
                }
                else
                {
                    perror("read from server");
                }
                running = 0;
                break;
            }
            else
            {
                server_buffer[bytes_read] = '\0';
                printf("Server: %s\n", server_buffer);
                printf("> ");
                fflush(stdout);
            }
        }

        // Check if user typed something
        if (fds[1].revents & POLLIN)
        {
            char buffer[1024];
            if (fgets(buffer, sizeof(buffer), stdin) == NULL)
            {
                printf("\nEOF detected. Exiting.\n");
                running = 0;
                break;
            }

            // Remove newline
            buffer[strcspn(buffer, "\n")] = '\0';

            if (strcmp(buffer, "EXIT") == 0)
            {
                printf("Exiting by user request.\n");
                running = 0;
                break;
            }

            if (send(sockfd, buffer, strlen(buffer), 0) < 0)
            {
                perror("send");
                running = 0;
                break;
            }

            printf("> ");
            fflush(stdout);
        }

        // Check for socket errors or hangup
        if (fds[0].revents & (POLLHUP | POLLERR))
        {
            printf("Server disconnected.\n");
            running = 0;
            break;
        }
    }

    close(sockfd);
    printf("Disconnected from server.\n");
    return 0;
}