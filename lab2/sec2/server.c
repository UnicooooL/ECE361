/* Library */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <pthread.h>

/* define const */
#define MAX_NAME 256
#define MAX_DATA 1024
#define BUFFER_SIZE 1024
#define MAX_CLIENTS 100
#define MAX_CONNECTIONS 100
#define MAX_SESSION 5
#define SESSION_CAPACITY 16
/////////////////
#define INACTIVITY_TIMEOUT 120
#define CHECK_INTERVAL 10

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

/* Client structure */
typedef struct {
    int sockfd;
    char id[MAX_NAME];
    char password[MAX_DATA]; // For simplicity, storing passwords in plain text
    bool connection;
    char session_id[MAX_DATA];
    /////////////////////////////////
    time_t last_activity;  // Add this to track the last activity time
} client_t;

struct message {
    unsigned int type;
    unsigned int size;
    unsigned char source[MAX_NAME];
    unsigned char data[MAX_DATA];
};

struct session {
    char session_id[MAX_DATA];
    char clients_id[SESSION_CAPACITY][MAX_NAME];
};

/* function declare */
client_t client_init(char id[], char password[]);
void *client_handler(void *socket_desc);
int add_session(const char* session_id);
void list_clients_sessions(char *list, size_t max_size);
bool is_session_empty(const char* session_id);
void delete_session();
char rec_username[MAX_NAME];
char private_message[MAX_DATA];
/////////////////////////////////////
void check_inactivity();
void* timer_thread(void *arg);

/* user defined vars */
client_t clients[MAX_CLIENTS];
struct session sessions[MAX_SESSION];
int con_count = 0;

/* MAIN FUNCTION */
int main(int argc, char **argv){
    /* input error */
    if (argc != 2) {
        printf("Usage: %s <port>\n", argv[0]);
        return EXIT_FAILURE;
    }
    /* define clients and passw */
    clients[0] = client_init("jill", "123");
    clients[1] = client_init("amy", "234");
    /* define, get input */
    int port = atoi(argv[1]);
    int option = 1;
    int listenfd = 0, connfd = 0;
    struct sockaddr_in serv_addr;
    struct sockaddr_in cli_addr;

    /* Socket settings */
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(port);

    // set socket
    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option)) < 0) {
        perror("ERROR: setsockopt failed");
        return EXIT_FAILURE;
    }

    // Bind
    if (bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("ERROR: Socket binding failed");
        return EXIT_FAILURE;
    }

    // Listen
    if (listen(listenfd, 10) < 0) {
        perror("ERROR: Socket listening failed");
        return EXIT_FAILURE;
    }

    // open the channel
    printf("=== WELCOME TO THE CHATROOM ===\n");

    // //////////////////////////////////////
    // //create the timer thread
    // pthread_t timer_thread_id; 
    // if (pthread_create(&timer_thread_id, NULL, timer_thread, NULL) < 0) {
    //     perror("Failed to create timer thread");
    //     //continue;
    // }
    // ///////////////////////////////////////

    /* receive msg */
    while(1){
        socklen_t clilen = sizeof(cli_addr);
        connfd = accept(listenfd, (struct sockaddr*)&cli_addr, &clilen);

        // Check if max clients is reached
        if ((con_count + 1) == MAX_CONNECTIONS) {
            printf("Max connections reached. Rejected: ");
            close(connfd);
            continue;
        }

        /* thread */
        pthread_t tid;
        int* new_sock = malloc(sizeof(int));
        *new_sock = connfd;
        // create normal thread
        if (pthread_create(&tid, NULL, client_handler, (void*) new_sock) < 0) {
            perror("could not create thread");
            close(connfd);
            free(new_sock);
            continue;
        }

        //////////////////////////////////////
        //create the timer thread
        pthread_t timer_thread_id; 
        if (pthread_create(&timer_thread_id, NULL, timer_thread, NULL) < 0) {
            perror("Failed to create timer thread");
            continue;
        }
        ///////////////////////////////////////
    }

}


/* USER DEFINED FUNCTIONS */

/* init client */
client_t client_init(char id[], char password[]){
    client_t client;
    memcpy(client.id, id, strlen(id) + 1);
    memcpy(client.password, password, strlen(password) + 1);
    client.sockfd = 0;
    client.connection = false;
    return client;
}

/* handle client msg */
void *client_handler(void *socket_desc){
    int sock = *(int*)socket_desc;
    int read_size;
    struct message msg;
    while((read_size = recv(sock, (char *)&msg, BUFFER_SIZE, 0)) > 0){
        ///////////////////////////
        // update active time
        int index_;
        for(int i = 0; i < MAX_CLIENTS; i++){
            if(strcmp(clients[i].id, (char*)msg.source) == 0){
                index_ = i;
                break;
            }
        }
        // When a message is received from a client
        time(&clients[index_].last_activity);  
        ///////////////////////////////
        // if(msg.type != 0){
        //     printf("received message type: %d\n", msg.type);
        // }
        

        /* login */
        if(msg.type == LOGIN){
            for(int i = 0; i < MAX_CLIENTS; i++){
                // first login
                if(strcmp(clients[i].id, (char*)msg.source) == 0 && !clients[i].connection){
                    if(strcmp(clients[i].password, (char*)msg.data) == 0){
                        clients[i].connection = true;
                        clients[i].sockfd = sock;
                        struct message ack_msg;
                        ack_msg.type = LO_ACK;
                        
                        printf("Client %s connected\n", clients[i].id);
                        send(clients[i].sockfd, (char *)&ack_msg, sizeof(ack_msg), 0);
                    }
                    else{ // error handle
                        struct message nack_msg;
                        nack_msg.type = LO_NAK;
                        char response[] = "Error: wrong password\n";
                        strcpy(nack_msg.data, response);
                        send(sock, (char *)&nack_msg, sizeof(nack_msg), 0);
                    }
                }
                // already logged in
                else if(strcmp(clients[i].id, (char*)msg.source) == 0 && clients[i].connection){
                    struct message nack_msg;
                    nack_msg.type = LO_NAK;
                    char response[] = "Error: client already logged in\n";
                    strcpy(nack_msg.data, response);
                    send(sock, (char *)&nack_msg, sizeof(nack_msg), 0);
                }
            }
        }

        /* create new session */
        else if(msg.type == NEW_SESS){
            int index;
            for(int i = 0; i < MAX_CLIENTS; i++){
                if(strcmp(clients[i].id, (char*)msg.source) == 0 && clients[i].connection){
                    index = i;
                    break;
                }
            }
            if (add_session((char*)msg.data) >= 0) {
                // Successfully created a new session
                strncpy(clients[index].session_id, (char*)msg.data, MAX_DATA);
                // Send NS_ACK to client
                struct message ack_msg;
                ack_msg.type = NS_ACK;
                if (send(sock, &ack_msg, sizeof(ack_msg), 0) < 0) { // error
                    perror("Failed to send NS_ACK");
                }
            } 
        }

        /* join a session */
        else if(msg.type == JOIN){
            int index = 0;
            bool found = false;
            // find which client is this now
            for(int i = 0; i < MAX_CLIENTS; i++){
                if(strcmp(clients[i].id, (char*)msg.source) == 0 && clients[i].connection){
                    index = i;
                    break;
                }
            }
            // find which session user wanna join
            for(int i = 0; i < MAX_SESSION; i++){
                if (strcmp(sessions[i].session_id, (char*)msg.data) == 0){
                    strncpy(clients[index].session_id, (char*)msg.data, MAX_DATA);
                    struct message ack_msg;
                    ack_msg.type = JN_ACK;
                    if (send(sock, &ack_msg, sizeof(ack_msg), 0) < 0) {
                        perror("Failed to send JN_ACK");
                        break;
                    }
                    found = true;
                    break;
                }
            }
            // error: didnt find the session
            if(!found){
                struct message nack_msg;
                nack_msg.type = JN_NAK;
                char response[] = "Error: Session ID does not exist\n";
                strcpy(nack_msg.data, response);
                if (send(sock, &nack_msg, sizeof(nack_msg), 0) < 0) {
                    perror("Failed to send JN_NAK");
                }
            }
        }

        /* leave the session */
        else if(msg.type == LEAVE_SESS){
            int index = 0;
            // find user
            for(int i = 0; i < MAX_CLIENTS; i++){
                if(strcmp(clients[i].id, (char*)msg.source) == 0 && clients[i].connection){
                    index = i;
                    break;
                }
            }
            clients[index].session_id[0] = '\0';
        }

        /* log out */
        else if(msg.type == EXIT){
            int index;
            // find user
            for(int i = 0; i < MAX_CLIENTS; i++){
                if(strcmp(clients[i].id, (char*)msg.source) == 0 && clients[i].connection){
                    index = i;
                    break;
                }
            }
            printf("Client %s disconnected\n", clients[index].id);
            clients[index].connection = false;
            close(sock);
        }

        /* print client - session list */
        else if(msg.type == QUERY){
            struct message ack_msg;
            ack_msg.type = QU_ACK;

            // Create the list of clients and sessions
            char list[MAX_DATA];
            memset(list, 0, MAX_DATA); // Ensure the list is empty
            list_clients_sessions(list, MAX_DATA);

            // Copy the list into the ack_msg data
            strncpy((char*)ack_msg.data, list, MAX_DATA);
            ack_msg.size = strlen((char*)ack_msg.data); // Set the size of the data

            // error handle
            if (send(sock, &ack_msg, sizeof(ack_msg), 0) < 0) {
                perror("Failed to send QUERY response");
            }
        }

        /* send msg */
        else if(msg.type == MESSAGE){
            int index;
            for(int i = 0; i < MAX_CLIENTS; i++){
                if(strcmp(clients[i].id, (char*)msg.source) == 0 && clients[i].connection){
                    index = i;
                    break;
                }
            }
            if(strlen(clients[index].session_id) > 0){
                for(int i = 0; i < MAX_CLIENTS; i++){
                    if(strcmp(clients[index].session_id, clients[i].session_id) == 0 && index != i 
                    && clients[i].connection){
                        if(send(clients[i].sockfd, &msg, sizeof(msg), 0) < 0){
                            perror("Sending message to other clients failed");
                        }else{
                            printf("Send %s: %s from %s in session %s\n", clients[i].id,  msg.data, clients[index].id, clients[index].session_id);
                        }
                        ////////////
                        

                    }
                }
            }
        }

        /* PRIVATE MSG */
        else if(msg.type == PR_MSG){
            if(sscanf(msg.data, "%s %s", &rec_username, &private_message) != 2){
                printf("invalid private message\n");
            }
            for(int i = 0; i < MAX_CLIENTS; i++){
                if(strcmp(rec_username, clients[i].id) == 0 && clients[i].connection){
                    struct message response;
                    response.type = PR_MSG;
                    strcpy(response.data, private_message);
                    strcpy(response.source, msg.source);
                    send(clients[i].sockfd, (char *)&response, sizeof(response), 0);
                }
            }
        }
    }
    delete_session();
}

/* create  new session */
int add_session(const char* session_id) {
    // Look for an existing session with the same id or an empty slot
    for (int i = 0; i < MAX_SESSION; i++) {
        if (strcmp(sessions[i].session_id, session_id) == 0) {
            // Session already exists
            return -1;
        } else if (strlen(sessions[i].session_id) == 0) {
            // Found an empty slot, create new session here
            strncpy(sessions[i].session_id, session_id, MAX_DATA);
            // Initialize other session attributes here if necessary
            return i; // Return the index of the new session
        }
    }
    return -2; // No space left for new sessions
}

/* Function to determine if a session has no clients */
bool is_session_empty(const char* session_id) {
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        if (clients[i].connection && strcmp(clients[i].session_id, session_id) == 0) {
            return false; // Found a client that is part of the session
        }
    }
    return true; // No clients found for this session
}

/* if empty, delete */
void delete_session(){
    for (int i = 0; i < MAX_SESSION; ++i) {
        if (strlen(sessions[i].session_id) > 0) {
            if (is_session_empty(sessions[i].session_id)) {
                sessions[i].session_id[0] = '\0';
                printf("Session deleted.\n");
            }
        }
    }
}

/* client - session list */
void list_clients_sessions(char *list, size_t max_size) {
    size_t offset = 0;

    // List all active sessions
    for (int i = 0; i < MAX_SESSION; i++) {
        if (strlen(sessions[i].session_id) > 0) { // Assuming an empty session_id means an inactive session
            int n = snprintf(list + offset, max_size - offset, "Session: %s\n", sessions[i].session_id);
            if (n > 0) offset += n; // Update the offset
        }
    }

    // List all connected clients
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].connection) { // Assuming 'connection' indicates an active client
            int n = snprintf(list + offset, max_size - offset, "Client: %s, Session: %s\n",
                             clients[i].id, clients[i].session_id);
            if (n > 0) offset += n; // Update the offset
        }
    }
    // error handle
    if (offset >= max_size) {
        perror("list clients and sessions exceed");
    }
}

////////////////////////////////////////////////
/* check inactive user */
void check_inactivity() {
    time_t now;
    double seconds;
    // check every client
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].connection) {
            time(&now);
            seconds = difftime(now, clients[i].last_activity);
            // check timer
            if (seconds >= INACTIVITY_TIMEOUT) {  
                // // Disconnect the client
                // struct message msg;
                // msg.type = INACT;  // Use the message type for inactivity warning
                // strcpy(msg.source, clients[i].id);
                // strcpy(msg.data, "Disconnected due to inactivity. Please login again.\n");
                // msg.size = strlen(msg.data);

                // // Send the warning message
                // send(clients[i].sockfd, (char *)&msg, sizeof(msg), 0);
                printf("Client %s disconnected due to inactivity.\n", clients[i].id);
                clients[i].connection = false;  // Mark as disconnected
                close(clients[i].sockfd);
            }
            // 30s left
            else if (seconds == INACTIVITY_TIMEOUT - 30) {  // Send warning 30 seconds before logout
                struct message warning_msg;
                warning_msg.type = INACT_WARN;  // Use the message type for inactivity warning
                strcpy(warning_msg.source, clients[i].id);
                strcpy(warning_msg.data, "You will be disconnected due to inactivity in 30 seconds. \nPlease enter command to active your account:");
                warning_msg.size = strlen(warning_msg.data);
                // Send the warning message
                send(clients[i].sockfd, (char *)&warning_msg, sizeof(warning_msg), 0);
            }


            
        }
    }

}

/* timer's thread handle */
void* timer_thread(void *arg) {
    while (1) {
        check_inactivity();
        // Convert to human-readable format and print
        sleep(CHECK_INTERVAL);  // after period, check
    }
}



