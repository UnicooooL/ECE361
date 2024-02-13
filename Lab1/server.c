/* Include packages */
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

    /* get the server's address */
    /* char *input = argv[1];
    if(argv[1]){
        int print_to_net = inet_pton(argv[1]);
    }else{
        int hum_to_net = getaddrinfo(argv[1]);
    } */

    /* Create a UDP socket */
    int socket_FD = socket(AF_INET, SOCK_DGRAM, 0);
    // corner case
    if (socket_FD < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    /* get the server address information */
    struct sockaddr_in server_addr = {0}; // sth handle internet address; a struct
    server_addr.sin_family = AF_INET; // set address family; specify using IPv4
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY); // specify the IP addr for socket; htonl is to ensure correct byte order for network (host to network long)
    server_addr.sin_port = htons(atoi(argv[1])); // host to network short

    /* Binding the socket to the server address*/ 
    // associates socket with specific IP and port#
    int bind_result = bind(socket_FD, (const struct sockaddr *)&server_addr, sizeof(server_addr));

    //If bind fails, try different port by user
    if(bind_result < 0){
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

/* -------------------------------------------------------------------------- */
    /* Receive messages from clients */
    char buffer[1024];
    struct sockaddr_storage client_addr;
    socklen_t len = sizeof(client_addr);

    while (1) {
        int n = recvfrom(socket_FD, (char *)buffer, 1024, 0, (struct sockaddr *) &client_addr, &len); // receive msg from a socket
        buffer[n] = '\0'; // Null-terminate the string

        // Check the message and respond
        const char *msg;
        if (strcmp(buffer, "ftp") == 0) {
            msg = "yes";
            printf("%s", msg);
        } else {
            msg = "no";
        }
        
        sendto(socket_FD, msg, strlen(msg), 0, (const struct sockaddr *) &client_addr, len);
    }

    // Close the socket
    close(socket_FD);

    return 0;
}

