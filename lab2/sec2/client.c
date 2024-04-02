/* library */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

/* define const */
#define MAX_NAME 256
#define MAX_DATA 1024
#define BUFFER_SIZE 1024
#define MAX_SESSION 5
#define RECONNECT_DELAY 5

#define LOGIN 1
#define LO_ACK 2
#define LO_NAK 3
#define EXIT 4
#define JOIN 5
#define JN_ACK 6
#define JN_NAK 7
#define LEAVE_SESS 8
#define NEW_SESS 9
#define NS_ACK 10
#define MESSAGE 11
#define QUERY 12
#define QU_ACK 13
#define PR_MSG 14
#define INACT 15
#define INACT_WARN 16

/* msg struct */
struct message {
    unsigned int type;
    unsigned int size;
    unsigned char source[MAX_NAME];
    unsigned char data[MAX_DATA];
};

/* func declare */
int connect_to_server(char *server_ip, int server_port);
int send_message(int sock, struct message msg);
void *receive_messages(void* arg);
void reconnect();

/* var define */
pthread_t recv_thread;
int sock = -1;
int recv_thread_created = 0;
int logged_in = 0;
char server_ip[MAX_NAME];
int server_port;

/* MAIN FUNCTION */
int main(int argc, char *argv[]){
    // define
    //pthread_t recv_thread;
    char input[MAX_DATA];
    char command[MAX_NAME], username[MAX_NAME], password[MAX_NAME];
    char private_message[MAX_DATA], rec_username[MAX_NAME];
    char sessionID[MAX_DATA];

    /* create recv thread */
    // if (pthread_create(&recv_thread, NULL, receive_messages, NULL) != 0) {
    //     perror("Could not create receive thread");
    //     return -1;
    // }

    // send msg
    while(1){
        printf("Please enter command: \n");
        fgets(input, MAX_DATA, stdin);
    ///////////////////////////////////////////////
        /* quit */
        if(strncmp(input, "/quit",5) == 0) {
            if(logged_in){
                //sock = connect_to_server(server_ip, server_port); //@
                struct message msg;
                msg.type = EXIT;
                strcpy((char*)msg.source, username);
                msg.size = 0;
                send_message(sock, msg);

                close(sock);
            }
            printf("quit successfully\n");
            exit(1);
        }
    ////////////////////////////////////////
        // Parse the input
        if (sscanf(input, "/login %s %s %s %d", username, password, server_ip, &server_port) == 4) {
            // Connect to the server with the provided IP and port
            struct message msg;
            sock = connect_to_server(server_ip, server_port);
            if(sock < 0) {
                // Handle connection error
                continue;
            }

            msg.type = LOGIN;
            strcpy(msg.source, username);
            strcpy(msg.data, password);
            msg.size = strlen(msg.data);

            //Send the login 
            send_message(sock, msg);
            logged_in = 1;
            if(!recv_thread_created){
                if (pthread_create(&recv_thread, NULL, receive_messages, NULL) != 0) {
                    perror("Could not create receive thread");
                    return -1;
                }else{
                    recv_thread_created = 1;
                    
                }
            }
            
            sleep(1);
            continue;
        } 

        /* if didnt log in */
        if(!logged_in){
            printf("You are not currently logged in. Please logged in first\n");
            continue;
        }

        /* log out */
        if (strncmp(input, "/logout", 7) == 0){
            if(logged_in){
                //sock = connect_to_server(server_ip, server_port); //@
                struct message msg;
                msg.type = EXIT;
                strcpy((char*)msg.source, username);
                msg.size = 0;
                send_message(sock, msg);

                close(sock);
                sock = -1;
                logged_in = 0;
                pthread_cancel(recv_thread);
                recv_thread_created = 0;
                printf("Logout successfully\n");
            }
            else{
                printf("You are not currently logged in. \n");
            }

        /* create session */
        }else if (sscanf(input, "/createsession %s", &sessionID) == 1){
            //sock = connect_to_server(server_ip, server_port); //@

            struct message msg;
            msg.type = NEW_SESS;
            strcpy(msg.source, username);
            strcpy(msg.data, sessionID);
            msg.size = strlen(msg.data);
            send_message(sock, msg);

        /* join session */
        }else if(sscanf(input, "/joinsession %s", &sessionID) == 1){
            //sock = connect_to_server(server_ip, server_port); //@
            
                struct message msg;
                msg.type = JOIN;
                strcpy(msg.source, username);
                strcpy(msg.data, sessionID);
                msg.size = strlen(msg.data);
                send_message(sock, msg);
    
        /* leave session */
        }else if(strncmp(input, "/leavesession", 13) == 0){
            //sock = connect_to_server(server_ip, server_port); //@

            struct message msg;
            msg.type = LEAVE_SESS;
            strcpy(msg.source, username);
            msg.size = 0;
            send_message(sock, msg);

        /* print list */
        }else if (strncmp(input, "/list", 5) == 0){
            //sock = connect_to_server(server_ip, server_port); //@

            struct message msg;
            msg.type = QUERY;
            strcpy(msg.source, username);
            msg.size = 0;
            send_message(sock, msg);

        /* PRIVATE MSG */
        }else if (sscanf(input, "/privatemsg %s %s", &rec_username, &private_message) == 2){
            //sock = connect_to_server(server_ip, server_port); //@
            struct message msg;
            msg.type = PR_MSG;
            snprintf(msg.data, MAX_DATA, "%s %s", rec_username, private_message);
            msg.size = strlen(msg.data);
            strcpy(msg.source, username);
            send_message(sock, msg);

        /* normal msg */
        }else{
            //sock = connect_to_server(server_ip, server_port); //@
            struct message msg;
            msg.type = MESSAGE;
            strcpy(msg.source, username);
            strcpy(msg.data, input);
            msg.size = strlen(msg.data);
            send_message(sock, msg);
        }

        /*after log in */
        if(logged_in){
            
           if(!recv_thread_created){
                if (pthread_create(&recv_thread, NULL, receive_messages, NULL) != 0) {
                    perror("Could not create receive thread");
                    return -1;
                }else{
                   recv_thread_created = 1; 
                }
                
           }
        }

        sleep(1);
    }

}

/* USER DEFINED FUNCTION */

/* connect server */
int connect_to_server(char *server_ip, int server_port) {
    int sock;
    struct sockaddr_in server;

    // Create socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("Could not create socket");
        return -1;
    }

    server.sin_addr.s_addr = inet_addr(server_ip);
    server.sin_family = AF_INET;
    server.sin_port = htons(server_port);

    // Connect to remote server
    if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("connect failed");
        return -1;
    }

    return sock;
}

/* send msg to server */
int send_message(int sock, struct message msg) {
    //printf("message sent\n");
    if(send(sock, &msg, sizeof(msg), 0) < 0) {
        perror("Send failed");
        return -1;
    }
    return 0;
}

/* recv msg from server */
void *receive_messages(void *arg) {
    char buffer[BUFFER_SIZE];
    struct message msg;
    while(1) {
        int len = recv(sock, (char *)&msg, BUFFER_SIZE, 0);
        if (len <= 0) {
            printf("You have been disconnected from the server.\n");
            pthread_cancel(recv_thread);
            //pthread_exit(NULL);
            logged_in = 0;
            recv_thread_created = 0;
        }
    
        // handle cases
        switch(msg.type){
            case LO_ACK:
                printf("Successful login.\n");
            break;
            case LO_NAK:
                printf("%s", msg.data);
            break;
            case NS_ACK:
                printf("Successfully create and join session.\n");
            break;
            case QU_ACK:
                printf("List of users and sessions available: \n");
                printf("%s", msg.data);
            break;
            case JN_ACK:
                printf("Join session successfully\n");
            break;
            case JN_NAK:
                printf("%s", msg.data);
            break;
            case MESSAGE:
                printf("%s: %s", msg.source, msg.data);
            break;
            case PR_MSG:
                printf("%s: %s\n", msg.source, msg.data);
                fflush(stdout);
            break;
            ///////////////////////////////////////////
            case INACT_WARN:
                printf("%s: %s\n", msg.source, msg.data); 
                fflush(stdout);  // Make sure the warning message is printed immediately
            break;
            // case INACT:
            //     printf("%s: %s\n", msg.source, msg.data); 
            //     logged_in = 0;
            //     sock = connect_to_server(server_ip, server_port);
            //     close(sock);
            //     sock = -1;
            // break;
        }
    }

}

