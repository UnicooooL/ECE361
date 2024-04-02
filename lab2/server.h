#ifndef SERVER_H
#define SERVER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>

#define MAX_CLIENTS 10

// Struct to hold client information
typedef struct client{
    int sockfd;
    struct sockaddr_in address;
    int addr_len;
    char* client_id;
    char* passward;
    char* session_id;
    char* IP;
    int port;
} Client;


// Struct to hold session information
typedef struct session{
    char* session_id;
    Client clients[MAX_CLIENTS];
} Session;


// Function to handle each client
void* handle_client(void* arg) {
    Client client = *(Client*)arg;

    // Perform operations with client.sockfd, like read/write

    // Close the client socket and end the thread when done
    close(client.sockfd);
    free(arg);
    pthread_exit(NULL);
}





#endif