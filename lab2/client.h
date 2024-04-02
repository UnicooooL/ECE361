#ifndef CLIENT_H
#define CLIENT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <regex.h>

#define MAX_NAME 100
#define MAX_DATA 1000
#define BUFFER_SIZE 1024


int serverPort = -1;
char* serverIp = NULL;
int loggedIn = 0; // false
char* currentSessionId = NULL;
char* currentcliendId = NULL;

struct message {
    unsigned int type;
    unsigned int size;
    unsigned char source[MAX_NAME];
    unsigned char data[MAX_DATA];
};

enum MessageType {
    LOGIN = 0,   
    LO_ACK = 1,      
    LO_NAK = 2,      
    EXIT = 3,
    JOIN = 4,
    JN_ACK = 5,
    JN_NAK = 6,
    LEAVE_SESS = 7,
    NEW_SESS = 8,
    NS_ACK = 9,
    MESSAGE = 10,
    QUERY = 11,
    QU_ACK = 12
};


// Function declarations
int connect_to_server(const char* ip, int port);
void* receive_messages(void* socket_desc);
int command_to_message(char* command, struct message* msg);
int login(char* command, struct message* msg);
void MtoS(const struct message *msg, void *result);
void StoM(const void* str, struct message *msg);


int connect_to_server(const char* ip, int port) {
    /* Create a TCP socket */
    int socket_FD = socket(AF_INET, SOCK_STREAM, 0);
    // corner case
    if (socket_FD < 0){
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    /* get the server's address */
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET; // set address family; specify using IPv4
    server_addr.sin_port = htons(port); // host to network short

    // convert pirntable to network form
    int print_to_net = inet_pton(AF_INET, ip, &server_addr.sin_addr); // (format, data); here is  ':'; int 1 is success, 0 is wrong string, -1 is error
    // if fail
    if(print_to_net != 1){ 
        fprintf(stderr, "network form fail\n");
            exit(EXIT_FAILURE);
    }

    // Connect to server
    /*
    int connect_to_serv = connect(socket_FD, (struct sockaddr*)&server_addr, sizeof(server_addr));
    //if fail
    if (connect_to_serv < 0) {
        perror("Connection failed");
        exit(EXIT_FAILURE);
    }
*/
    printf("Connected to server at %s:%d\n", ip, port);
    return socket_FD;
}


void* receive_messages(void* socket_desc) {
    int sockfd = *(int*)socket_desc;
    char buffer[BUFFER_SIZE];
    ssize_t message_len;
    struct message* msg;

    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        message_len = recv(sockfd, buffer, BUFFER_SIZE, 0);
        StoM(buffer, msg);
        
        if (message_len > 0) {
            printf("Server: %s\n", buffer);
        } else if (message_len == 0) {
            puts("Server connection closed.");
            break;
        } else {
            perror("recv failed");
            break;
        }
    }
    
    return 0;
}



// Function to convert command to message struct
int command_to_message(char* command, struct message* msg) {
    // Ensure msg is not NULL
    if (msg == NULL) {
        return -1; // return an error
    }

    // Start with a clear message structure
    memset(msg, 0, sizeof(struct message));
    strncpy(msg->source, currentcliendId, MAX_NAME);

    char* rest = NULL;
    char* command_cpy = strdup(command);
    if (command_cpy == NULL) {
        
        printf("failed copy\n");
        return 0;
    }
    //strcpy(command_cpy, command);
    char* instr = strtok_r(command_cpy, " ", &rest); // instruction
    
    if (strcmp(instr, "/logout") == 0) {
        char* token = strtok_r(NULL, " ", &rest); 
        if(token != NULL){
            printf("Error logout. Try again.\n");
            return -2;
        }
        // Set the message type
        msg->type = EXIT;
        serverPort = -1;
        serverIp = NULL;
        loggedIn = 0; // false
        currentSessionId = NULL;
        char* pre_client = currentcliendId;
        currentcliendId = NULL;
        printf("Successful logout: %s.\n", pre_client);
    }else if(strcmp(instr, "/joinsession") == 0){
        // Set the message type
        msg->type = JOIN;

        // Tokenize the command to extract the parameters
        char* session_id = strtok_r(NULL, " ", &rest); // session_id
        if (session_id != NULL) {
            strncpy(msg->data, session_id, MAX_DATA);
            msg->size = strlen(session_id);
            currentSessionId = session_id;
            printf("Successful joinsession: %s.\n", currentSessionId);
        }else{
            printf("Error session_id joinsession. Try again.\n");
            return -2;
        }
    }else if(strcmp(instr, "/leavesession") == 0){
        // Set the message type
        char* token = strtok_r(NULL, " ", &rest); 
        if(token != NULL){
            printf("Error leavesession. Try again.\n");
            return -2;
        }
        msg->type = LEAVE_SESS;
        char* pre_session = currentSessionId;
        currentSessionId = NULL;
        printf("Successful leavesession: %s.\n", pre_session);
    }else if(strcmp(instr, "/createsession") == 0){
        // Set the message type
        msg->type = NEW_SESS;

        // Tokenize the command to extract the parameters
        char* session_id = strtok_r(NULL, " ", &rest); // session_id
        if (session_id != NULL) {
            strncpy(msg->data, session_id, MAX_DATA);
            msg->size = strlen(session_id);
            currentSessionId = session_id;
            printf("Successful createsession: %s.\n", currentSessionId);
        }else{
            printf("Error session_id createsession. Try again.\n");
            return -2;
        }
    }else if(strcmp(instr, "/list") == 0){
        char* token = strtok_r(NULL, " ", &rest); 
        if(token != NULL){
            printf("Error list. Try again.\n");
            return -2;
        }
        // Set the message type
        msg->type = QUERY;
        printf("Successful list.\n");

    }else{
        // Set the message type
        msg->type = MESSAGE;

        strncpy(msg->data, command, MAX_DATA);
        msg->size = strlen(command);
        printf("Successful message: %s.\n", msg->data);
    }

    return 0; // return success
}

int login(char* command, struct message* msg){
    // Set the message type
    msg->type = LOGIN;
    // Start with a clear message structure
    memset(msg, 0, sizeof(struct message));

    // Tokenize the command to extract the parameters
    char* rest = NULL;

    char* command_cpy = strdup(command);
    if (command_cpy == NULL) {
        
        printf("failed copy\n");
        return 0;
    }
    //strcpy(command_cpy, command);
    char* token = strtok_r(command_cpy, " ", &rest);// /login

    char* client_id = strtok_r(NULL, " ", &rest); //client_id
    if (client_id != NULL) {
        strncpy(msg->source, client_id, MAX_NAME);
        currentcliendId = client_id;
    }else{
        printf("Error client_id login. Try again.\n");
        return 0;
    }


    char* password = strtok_r(NULL, " ", &rest); //Password
    if (password != NULL) {
        strncpy(msg->data, password, MAX_DATA);
        msg->size = strlen(password);
    } else {
        printf("Error password login. Try again.\n");
        return 0;
    }


    serverIp = strtok_r(NULL, " ", &rest); //server_ip
    if (serverIp == NULL) {
        printf("Error server_ip login. Try again.\n");
        return 0;
    }

    char* server_port_str = strtok_r(NULL, " ", &rest); //server_port
    if (server_port_str == NULL) {
        printf("Error server_port login. Try again.\n");
        return 0;
    }else{
        serverPort = atoi(server_port_str);
    }
    
    return 1;


}

/* User defined */
void MtoS(const struct message *msg, void *result) {
    sprintf(result, "%d:%d:%s:", msg->type, msg->size, msg->source);
    memcpy(result + strlen(result), msg->data, msg->size);
}


void StoM(const void* str, struct message *msg) {

    // Compile Regex to match ":"
    regex_t regex;
    if(regcomp(&regex, "[:]", REG_EXTENDED)) {
        fprintf(stderr, "Could not compile regex\n");
    }

    // Match regex to find ":" 
    regmatch_t pmatch[1];
    int target = 0;
    char buf[BUFFER_SIZE];

    // Match type
    if(regexec(&regex, str + target, 1, pmatch, REG_NOTBOL)) {
        fprintf(stderr, "Error matching regex\n");
        exit(1);
    }
    memset(buf, 0, BUFFER_SIZE * sizeof(char));
    memcpy(buf, str + target, pmatch[0].rm_so);
    msg -> type = atoi(buf);
    target += (pmatch[0].rm_so + 1);


    // Match size
    if(regexec(&regex, str + target, 1, pmatch, REG_NOTBOL)) {
        fprintf(stderr, "Error matching regex\n");
        exit(1);
    }
    memset(buf, 0, BUFFER_SIZE * sizeof(char));
    memcpy(buf, str + target, pmatch[0].rm_so);
    msg -> size = atoi(buf);
    target += (pmatch[0].rm_so + 1);

    // Match source
    if(regexec(&regex, str + target, 1, pmatch, REG_NOTBOL)) {
        fprintf(stderr, "Error matching regex\n");
        exit(1);
    }

    //memset(msg -> filename, 0, BUFFER * sizeof(char));
    memcpy(msg -> source, str + target, pmatch[0].rm_so);

    msg -> source[pmatch[0].rm_so] = 0;
    target += (pmatch[0].rm_so + 1);
    
    // Match filedata
    memcpy(msg -> data, str + target, msg -> size);

}

#endif
