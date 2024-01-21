// Include packages
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <UDP listen port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Create a UDP socket
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Binding the socket to the server address
    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET; // IPv4
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(atoi(argv[1]));

    if (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // Receive messages from clients
    char buffer[1024];
    struct sockaddr_in cliaddr;
    int len = sizeof(cliaddr);

    while (1) {
        int n = recvfrom(sockfd, (char *)buffer, 1024, MSG_WAITALL, (struct sockaddr *) &cliaddr, &len);
        buffer[n] = '\0'; // Null-terminate the string.

        // Check the message and respond
        const char *msg;
        if (strcmp(buffer, "ftp") == 0) {
            msg = "yes";
        } else {
            msg = "no";
        }
        sendto(sockfd, msg, strlen(msg), MSG_CONFIRM, (const struct sockaddr *) &cliaddr, len);
    }

    // Close the socket
    close(sockfd);

    return 0;
}

