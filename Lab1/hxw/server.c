#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 1100
#define DATA_SIZE 1000

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <UDP listen port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int sockfd;
    struct sockaddr_in server_addr, client_addr;
    char buffer[BUFFER_SIZE];
    socklen_t addr_len;

    // Create a UDP socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Set server address and bind socket
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(atoi(argv[1]));

    if (bind(sockfd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    printf("Server is listening on port %s\n", argv[1]);

    // Listen for messages
    addr_len = sizeof(client_addr);
    FILE *file = NULL;
    unsigned int expected_frag_num = 1;

    while(1){
        memset(buffer, 0 , BUFFER_SIZE);
        if(recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr *) &client_addr, &addr_len) < 0){
            perror("Error in recvfrom in server.c");
        }
        perror("Checkpoint 1");

        unsigned int total_frag, frag_num, size;
        char filename[100];
        sscanf(buffer, "%u:%u:&u:&s:", &total_frag, &frag_num, &size, filename);
        perror("Checkpoint 2");

        if(frag_num == 1){
            file = fopen(filename, "wb");
            if(!file){
                perror("Error in openning file for writing");
                exit(1);
            }
        }

        if(frag_num == expected_frag_num){
            fwrite(buffer + strlen(buffer) + 1, 1, size, file);
            expected_frag_num ++;
        }

        char ack[20];
        sprintf(ack, "ACK: fragment %u", frag_num);
        sendto(sockfd, ack, strlen(ack), 0, (struct sockaddr *) &client_addr, addr_len);

        if(frag_num == total_frag){
            fclose(file);
            file = NULL;
            expected_frag_num = 1;
        }
    }

    close(sockfd);
    return 0;
}
