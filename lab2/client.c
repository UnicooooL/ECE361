#include "client.h"

int main(int argc, char *argv[]){
    // Start a thread to listen for messages from the server
    pthread_t recv_thread;
    

    // Command input loop
    char command[BUFFER_SIZE];;
    
    int socket_FD;
    int result = -2;

    
    while(1) {
        if(fgets(command, BUFFER_SIZE, stdin) == NULL){
            printf("Error input size. Try again.\n");
            continue;
        }
        
        // Remove newline character
        command[strcspn(command, "\n")] = 0;

        // Start with a clear message structure
        struct message msg;
        memset(&msg, 0, sizeof(msg));

        if(strcmp(command, "/quit") == 0) {
            break;
        }

        // LOGIN command
        if (strncmp(command, "/login", 6) == 0) {
            if(!loggedIn){
                if(login(command, &msg)){
                    socket_FD = connect_to_server(serverIp, serverPort);
                    loggedIn = 1;
                    printf("Successful login %s.\n", currentcliendId);
                    if(pthread_create(&recv_thread, NULL, receive_messages, (void*)&socket_FD) < 0) {
                        perror("Could not create thread");
                        return 1;
                    }
                }else{
                    printf("Error login. Try again.\n");
                    continue;
                }
            }else{
                printf("Already login. Try again.\n");
                continue;
            }  
        }else if(loggedIn){
            result = command_to_message(command, &msg);
        
            
        }else{
            printf("Please login. Try again.\n");
            continue;
        }


        int send_to_serv = -1; 
        if(result == 0){
            char buffer[BUFFER_SIZE];
            MtoS(&msg, buffer);
            //send_to_serv = send(socket_FD, buffer, sizeof(buffer), 0);
        }
        /*
        if (send_to_serv < 0) {
            perror("Send failed");
            exit(EXIT_FAILURE);
        }*/

        
        
        
        
    }

    // Cleanup
    pthread_cancel(recv_thread);
    close(socket_FD);
    return 0;
}