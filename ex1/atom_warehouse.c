#include <poll.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define EXIT_FAILURE 1
#define BACKLOG 5
#define MAX_CLIENTS 100

typedef struct wareHouse
{
    unsigned long long carbon;
    unsigned long long hydrogen;
    unsigned long long oxygen;
} wareHouse;

void addAtom(int atom, int quantity, wareHouse *warehouse)
{
    switch (atom)
    {
    case 1:
        warehouse->carbon += quantity;
        break;
    case 2:
        warehouse->hydrogen += quantity;
        break;
    case 3:
        warehouse->oxygen += quantity;
        break;
    default:
        printf("you don't know how to add atoms\n");
        break;
    }
}

void printAtoms(wareHouse *warehouse)
{
    printf("Carbon: %llu\n", warehouse->carbon);
    printf("Hydrogen: %llu\n", warehouse->hydrogen);
    printf("Oxygen: %llu\n", warehouse->oxygen);
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Usage: %s <port>\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *atoms[] = {"CARBON", "HYDROGEN", "OXYGEN"};
    size_t size_atoms = sizeof(atoms) / sizeof(atoms[0]);
    wareHouse warehouse = {0};

    int port = atoi(argv[1]);

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        perror("socket");
        return EXIT_FAILURE;
    }

    struct sockaddr_in server_addr = {
        .sin_family = AF_INET,
        .sin_port = htons(port),
        .sin_addr.s_addr = INADDR_ANY};

    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        perror("bind");
        close(sockfd);
        return EXIT_FAILURE;
    }

    if (listen(sockfd, BACKLOG) == -1)
    {
        perror("listen");
        close(sockfd);
        return EXIT_FAILURE;
    }

    struct pollfd fds[MAX_CLIENTS];
    int nfds = 1;

    fds[0].fd = sockfd;
    fds[0].events = POLLIN;

    printf("Warehouse server running on port %d...\n", port);

    while (1)
    {
        int poll_count = poll(fds, nfds, -1);
        if (poll_count < 0)
        {
            perror("poll");
            break;
        }

        for (int i = 0; i < nfds; i++)
        {
            if (fds[i].revents & POLLIN)
            {
                if (fds[i].fd == sockfd)
                {
                    // חיבור חדש
                    int new_fd = accept(sockfd, NULL, NULL);
                    if (new_fd >= 0 && nfds < MAX_CLIENTS)
                    {
                        fds[nfds].fd = new_fd;
                        fds[nfds].events = POLLIN;
                        nfds++;
                        printf("New client connected: fd=%d\n", new_fd);
                    }
                }
                else
                {
                    // הודעה מלקוח קיים
                    char buffer[1024];
                    ssize_t bytes_received = recv(fds[i].fd, buffer, sizeof(buffer) - 1, 0);

                    if (bytes_received <= 0)
                    {
                        printf("Client disconnected: fd=%d\n", fds[i].fd);
                        close(fds[i].fd);
                        fds[i] = fds[nfds - 1];
                        nfds--;
                        i--;
                        continue;
                    }

                    buffer[bytes_received] = '\0';
                    printf("Received message: %s", buffer);

                    char atom[16];
                    int quantity;

                    if (sscanf(buffer, "ADD %15s %d", atom, &quantity) == 2)
                    {
                        int atom_index = 0;
                        for (size_t j = 0; j < size_atoms; j++)
                        {
                            if (strcmp(atom, atoms[j]) == 0)
                            {
                                atom_index = j + 1;
                                break;
                            }
                        }

                        if (atom_index != 0)
                        {
                            if (quantity > 0)
                            {
                                addAtom(atom_index, quantity, &warehouse);
                                printf("Added %d %s atoms\n", quantity, atom);
                                printAtoms(&warehouse);
                            }
                            else 
                            {
                                printf("Invalid quantity: %d\n", quantity);
                            }
                        }
                        else
                        {
                            printf("Invalid atom type: %s\n", atom);
                        }
                    }
                    else
                    {
                        printf("Invalid command format\n");
                    }
                }
            }
        }
    }

    for (int i = 0; i < nfds; i++)
    {
        close(fds[i].fd);
    }

    return 0;
}
