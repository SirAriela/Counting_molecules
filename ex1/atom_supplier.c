#include <poll.h>
#include <unistd.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h> 
#include <arpa/inet.h> 


#define EXIT_FAILURE 1

int main(int argc, char *argv[]) {
   if (argc != 3) {
        fprintf(stderr, "Usage: %s <server_ip> <port>\n", argv[0]);
        return 1;
    }

    const char *server_ip = argv[1];
    int port = atoi(argv[2]);


    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket");
        return 1;
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0) {
        perror("inet_pton");
        close(sockfd);
        return 1;
    }

    // התחברות לשרת
    if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect");
        close(sockfd);
        return 1;
    }

    printf("Connected to warehouse server at %s:%d\n", server_ip, port);

    //send a message to the server
    char buffer[1024];
    fgets(buffer, sizeof(buffer), stdin);

    ssize_t bytes_sent = send(sockfd, buffer, sizeof(buffer), 0);
    if (bytes_sent < 0) {
        perror("send");
        close(sockfd);
        return 1;
    }
    printf("Sent message to server: %s\n", buffer);

    // //receive a message from the server
    // struct pollfd fds[1];
    // fds[0].fd = sockfd;
    // fds[0].events = POLLIN;
    // int poll_count = poll(fds, 1, -1);
    // if (poll_count < 0) {
    //     perror("poll");
    //     close(sockfd);
    //     return 1;
    // }
    // if (fds[0].revents & POLLIN) {
    //     ssize_t bytes_received = recv(sockfd, buffer, sizeof(buffer), 0);
    //     if (bytes_received < 0) {
    //         perror("recv");
    //         close(sockfd);
    //         return 1;
    //     }
    //     printf("Received message from server: %s\n", buffer);
    // }
    //close the socket
    close(sockfd);
    return 0;
}