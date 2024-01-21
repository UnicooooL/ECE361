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

    //get the server's address
    char input[] = argv[1];
    if(argv[1])
    int inet_pton(argv[1]);

    // Create a UDP socket
    int socket_FD = socket(AF_INET, SOCK_DGRAM, 0);

    if (socket_FD < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    //get the server address information 
    struct sockaddr_in server_addr = {0};
    server_addr.sin_family = AF_INET; // IPv4
    server_addr.sin_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(inet_pton(argv[1]));
    

    // Binding the socket to the server address
    int bind_result = bind(socket_FD, (const struct sockaddr *)&server_addr, sizeof(server_addr));

    //If bind fails
    if(bind_result < 0){
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // Receive messages from clients
    char buffer[1024];
    struct sockaddr_in cliaddr;
    int len = sizeof(cliaddr);

    while (1) {
        int n = recvfrom(socket_FD, (char *)buffer, 1024, MSG_WAITALL, (struct sockaddr *) &cliaddr, &len);
        buffer[n] = '\0'; // Null-terminate the string.

        // Check the message and respond
        const char *msg;
        if (strcmp(buffer, "ftp") == 0) {
            msg = "yes";
        } else {
            msg = "no";
        }
        sendto(socket_FD, msg, strlen(msg), MSG_CONFIRM, (const struct sockaddr *) &cliaddr, len);
    }

    // Close the socket
    close(socket_FD);

    return 0;
}