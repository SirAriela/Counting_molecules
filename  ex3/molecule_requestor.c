#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s <server_ip> <port>\n", argv[0]);
        return 1;
    }
    
    const char *server_ip = argv[1];
    int port = atoi(argv[2]);
    
    // Validate port
    if (port <= 0 || port > 65535)
    {
        fprintf(stderr, "Invalid port number: %d\n", port);
        return 1;
    }
    
    // Create UDP socket
    int udp_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (udp_socket < 0)
    {
        perror("socket");
        return 1;  // No need to close if socket creation failed
    }
    
    // Setup server address
    struct sockaddr_in server;
    socklen_t server_len = sizeof(server);
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    
    if (inet_pton(AF_INET, server_ip, &server.sin_addr) <= 0)
    {
        perror("inet_pton");
        close(udp_socket);
        return 1;
    }
    
    // Read input from user
    printf("Enter command (e.g., 'DELIVER WATER 2'): ");
    char buffer[1024];
    if (fgets(buffer, sizeof(buffer), stdin) == NULL)
    {
        fprintf(stderr, "Error reading input\n");
        close(udp_socket);
        return 1;
    }
    
    // Remove newline
    buffer[strcspn(buffer, "\n")] = 0;
    
    // Send message to server
    if (sendto(udp_socket, buffer, strlen(buffer), 0, (struct sockaddr *)&server, server_len) < 0)
    {
        perror("sendto");
        close(udp_socket);
        return 1;
    }
    
    // Receive response from server
    char response[1024];
    ssize_t len = recvfrom(udp_socket, response, sizeof(response) - 1, 0, NULL, NULL);
    if (len < 0)
    {
        perror("recvfrom");
        close(udp_socket);
        return 1;
    }
    
    response[len] = '\0';
    printf("Server response: %s\n", response);
    
    close(udp_socket);
    return 0;
}