#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <getopt.h>
#include <sys/poll.h>
#include <time.h>
#include <errno.h>

extern char *optarg;

void print_usage(const char *program_name)
{
    printf("Usage: %s [-h <host> -p <port>] OR [-f <socket_path>]\n", program_name);
    printf("Options:\n");
    printf("  -h <host>    Server hostname or IP address\n");
    printf("  -p <port>    Server port number\n");
    printf("  -f <path>    Unix Domain Socket path\n");
    printf("\nNote: Use either inet socket (-h and -p) OR unix socket (-f), not both\n");
}

int main(int argc, char *argv[])
{
    char *hostname = NULL;
    int port = -1;
    char *socket_path = NULL;
    int c;

    // Parse command line arguments
    while ((c = getopt(argc, argv, "h:p:f:")) != -1)
    {
        switch (c)
        {
        case 'h':
            hostname = optarg;
            break;
        case 'p':
            port = atoi(optarg);
            break;
        case 'f':
            socket_path = optarg;
            break;
        case '?':
            print_usage(argv[0]);
            return 1;
        }
    }

    // Validate arguments
    int has_inet = (hostname != NULL && port > 0);
    int has_unix = (socket_path != NULL);

    if (!has_inet && !has_unix)
    {
        fprintf(stderr, "Error: Must specify either inet socket (-h -p) or unix socket (-f)\n");
        print_usage(argv[0]);
        return 1;
    }

    if (has_inet && has_unix)
    {
        fprintf(stderr, "Error: Cannot use both inet socket and unix socket simultaneously\n");
        print_usage(argv[0]);
        return 1;
    }

    int sockfd;
    struct sockaddr_in serv_addr;
    struct sockaddr_un unix_addr;
    struct sockaddr *addr;
    socklen_t addr_len;

    if (has_inet)
    {
        // Create inet socket
        sockfd = socket(AF_INET, SOCK_DGRAM, 0);
        if (sockfd < 0)
        {
            perror("socket");
            return 1;
        }

        memset(&serv_addr, 0, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(port);

        if (inet_pton(AF_INET, hostname, &serv_addr.sin_addr) <= 0)
        {
            perror("inet_pton");
            close(sockfd);
            return 1;
        }

        addr = (struct sockaddr *)&serv_addr;
        addr_len = sizeof(serv_addr);
        printf("Using UDP connection to %s:%d\n", hostname, port);
    }
    else
    {
        // Create unix socket
        sockfd = socket(AF_UNIX, SOCK_DGRAM, 0);
        if (sockfd < 0)
        {
            perror("socket");
            return 1;
        }

        memset(&unix_addr, 0, sizeof(unix_addr));
        unix_addr.sun_family = AF_UNIX;
        strncpy(unix_addr.sun_path, socket_path, sizeof(unix_addr.sun_path) - 1);

        addr = (struct sockaddr *)&unix_addr;
        addr_len = sizeof(unix_addr);
        printf("Using Unix datagram socket: %s\n", socket_path);
    }

    char buffer[256];
    char response[256];
    time_t last_activity = time(NULL);
    int waiting_for_response = 0;

    printf("Enter commands (DELIVER <MOLECULE> <QUANTITY>). Type 'quit' to exit:\n");
    printf("> ");
    fflush(stdout);
    
    // Set up polling for socket and stdin
    struct pollfd fds[2];
    fds[0].fd = sockfd;
    fds[0].events = POLLIN;
    fds[1].fd = STDIN_FILENO;
    fds[1].events = POLLIN;

    printf("> ");
    fflush(stdout);

    while (1)
    {
        // Poll for events with timeout of 5 seconds
        int poll_result = poll(fds, 2, 5000);
        
        if (poll_result < 0)
        {
            perror("poll");
            break;
        }
        
        // Check for timeout - if we're waiting for response and got timeout
        if (poll_result == 0)
        {
            if (waiting_for_response && (time(NULL) - last_activity > 10))
            {
                printf("\nNo response from server for 10 seconds. Server may be down.\n");
                printf("Type 'quit' to exit or continue trying.\n");
                printf("> ");
                fflush(stdout);
                waiting_for_response = 0;
            }
            continue;
        }

        // Check if stdin has input
        if (fds[1].revents & POLLIN)
        {
            if (fgets(buffer, sizeof(buffer), stdin) == NULL)
            {
                break;
            }

            // Remove newline
            char *newline = strchr(buffer, '\n');
            if (newline)
                *newline = '\0';

            if (strcmp(buffer, "quit") == 0)
            {
                break;
            }

            // Send message to server
            if (sendto(sockfd, buffer, strlen(buffer), 0, addr, addr_len) < 0)
            {
                if (errno == ECONNREFUSED)
                {
                    printf("Connection refused - server appears to be down\n");
                    break;
                }
                perror("sendto");
                break;
            }
            
            waiting_for_response = 1;
            last_activity = time(NULL);
        }

        // Check if socket has data
        if (fds[0].revents & POLLIN)
        {
            // Receive response from server
            ssize_t recv_len = recvfrom(sockfd, response, sizeof(response) - 1, 0, NULL, NULL);
            if (recv_len > 0)
            {
                response[recv_len] = '\0';
                printf("Server response: %s\n", response);
                waiting_for_response = 0;
                last_activity = time(NULL);
            }
            else if (recv_len < 0)
            {
                if (errno == ECONNREFUSED)
                {
                    printf("Server connection refused - server appears to be down\n");
                    break;
                }
                perror("recvfrom");
                break;
            }
        }
        
        // Show prompt after processing events
        if (fds[1].revents & POLLIN || fds[0].revents & POLLIN)
        {
            printf("> ");
            fflush(stdout);
        }
    }

    close(sockfd);
    printf("Disconnected from server\n");
    return 0;
}