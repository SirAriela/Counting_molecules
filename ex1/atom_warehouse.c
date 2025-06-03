#include <poll.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>

#define BACKLOG 10
#define MAX_CLIENTS 10

volatile sig_atomic_t running = 1;

void handle_sigint(int sig) {
    running = 0;
    printf("\nSIGINT received — shutting down server gracefully... bli neder\n");
}

typedef struct wareHouse {
    unsigned long long carbon;
    unsigned long long hydrogen;
    unsigned long long oxygen;
} wareHouse;

void addAtom(int atom, int quantity, wareHouse *warehouse) {
    switch (atom) {
        case 1: warehouse->carbon += quantity; break;
        case 2: warehouse->hydrogen += quantity; break;
        case 3: warehouse->oxygen += quantity; break;
       
    }
}

void printAtoms(wareHouse *warehouse) {
    printf("Carbon: %llu\n", warehouse->carbon);
    printf("Hydrogen: %llu\n", warehouse->hydrogen);
    printf("Oxygen: %llu\n", warehouse->oxygen);

}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        return 1;
    }

    signal(SIGINT, handle_sigint);

    wareHouse warehouse = {0};
    const char *atoms[] = {"CARBON", "HYDROGEN", "OXYGEN"};
    int port = atoi(argv[1]);

    if (port <= 0 || port > 65535) {
        fprintf(stderr, "Invalid port number: %d\n", port);
        return 1;
    }
    // Create a listening socket
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0) {
        perror("socket");
        return 1;
    }

    // Set socket options to allow reuse of the address
    int opt = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in serv_addr = {
        .sin_family = AF_INET,
        .sin_port = htons(port),
        .sin_addr.s_addr = INADDR_ANY
    };

    if (bind(listen_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("bind");
        close(listen_fd);
        return 1;
    }

    if (listen(listen_fd, BACKLOG) < 0) {
        perror("listen");
        close(listen_fd);
        return 1;
    }

    struct pollfd fds[MAX_CLIENTS];
    int nfds = 1;
    fds[0].fd = listen_fd;
    fds[0].events = POLLIN;

    printf("Server running on port %d...\n", port);

    while (running) {
        int ready = poll(fds, nfds, 1000);
        if (ready < 0) {
            if (!running) break;
            perror("poll");
            break;
        }

        if (ready == 0) continue; // timeout, no events

        // Handle new connections first
        if (fds[0].revents & POLLIN) {
            int client_fd = accept(listen_fd, NULL, NULL);
            if (client_fd >= 0 && nfds < MAX_CLIENTS) {
                fds[nfds].fd = client_fd;
                fds[nfds].events = POLLIN;
                fds[nfds].revents = 0; // Clear revents
                nfds++;
                printf("New client connected: fd=%d\n", client_fd);
            } else if (client_fd >= 0) {
                printf("Max clients reached, rejecting connection\n");
                close(client_fd);
            }
        }

        // Handle client data - process from end to beginning to avoid index issues
        for (int i = nfds - 1; i >= 1; i--) {
            if (fds[i].revents & POLLIN) {
                char buffer[256];
                ssize_t len = read(fds[i].fd, buffer, sizeof(buffer) - 1);

                if (len <= 0) {
                    printf("Client disconnected: fd=%d\n", fds[i].fd);
                    close(fds[i].fd);
                    // Move last element to current position
                    if (i < nfds - 1) {
                        fds[i] = fds[nfds - 1];
                    }
                    nfds--;
                } else {
                    buffer[len] = '\0';
                    // Remove newline if present
                    char *newline = strchr(buffer, '\n');
                    if (newline) *newline = '\0';
                    
                    char atom[16];
                    int quantity = 0;
                    if (sscanf(buffer, "ADD %15s %d", atom, &quantity) == 2 && quantity > 0) {
                        int index_atom = -1;
                        for (int j = 0; j < 3; j++) {
                            if (strcmp(atom, atoms[j]) == 0) {
                                index_atom = j + 1;
                                break;
                            }
                        }
                        if (index_atom > 0) {
                            addAtom(index_atom, quantity, &warehouse);
                            printf("Added %d %s\n", quantity, atom);
                            printAtoms(&warehouse);
                        
                        }
                    } 
                }
            }
        }
        
        // Clear all revents for next iteration
        for (int i = 0; i < nfds; i++) {
            fds[i].revents = 0;
        }
    }
    // here only if running is false - signal CTRL C
    printf("Shutting down server...\n");
    for (int i = 1; i < nfds; i++) {
        close(fds[i].fd);
    }

    close(listen_fd);
    printf("Server terminated.\n");
    return 0;
}