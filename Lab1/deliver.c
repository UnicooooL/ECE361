/* Include packages */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netdb.h>
#define _OPEN_SYS_SOCK_IPV6
#include <arpa/inet.h>

int main(int argc, char *argv[]) {
   if (argc != 3) {
      fprintf(stderr, "Usage: %s <server address> <server port number>\n", argv[0]);
      exit(EXIT_FAILURE);
   }

   /* get the server's address */
   struct addrinfo* service_info; // store the returned info
   struct addrinfo hints; // declare
   struct sockaddr_in server_addr = {0};
   server_addr.sin_family = AF_INET; // set address family; specify using IPv4
   //server_addr.sin_addr.s_addr = htonl(INADDR_ANY); // specify the IP addr for socket; htonl is to ensure correct byte order for network (host to network long)
   server_addr.sin_port = htons(atoi(argv[2])); // host to network short

   // set up the hints
   memset(&hints, 0, sizeof hints); // 0 to ensure the struct empty
   hints.ai_family = AF_INET; // IPV4
   hints.ai_socktype = SOCK_STREAM; // TCP stream sockets
   hints. ai_flags = AI_PASSIVE; // fill in the IP 

   // convert pirntable to network form
   int print_to_net = inet_pton(AF_INET, argv[1], &server_addr.sin_addr); // (format, data); here is  ':'; int 1 is success, 0 is wrong string, -1 is error
   // testing
   /* char ipstr [INET6_ADDRSTRLEN];
   inet_ntop(AF_INET, &(server_addr.sin_addr), ipstr, sizeof ipstr);
   printf("IP address: %s", ipstr); */
   // if fail
   if(print_to_net != 1){ 
      fprintf(stderr, "network form fail\n");
         exit(EXIT_FAILURE);
   }
   /* if(print_to_net != 1){ // convert human-readable to network form
      int hum_to_net = getaddrinfo(argv[1], argv[2], &hints, &service_info);
      if(hum_to_net != 0){ // 0 is success
         fprintf(stderr, "network form fail: %s \n", gai_strerror(hum_to_net));
         exit(EXIT_FAILURE);
      }else{
         server_addr.sin_addr.s_addr = *(service_info->ai_addr->sa_data);
      }
   } */

   /* prompt user input */
   char input[1024];
   char command[4];  // Buffer for the command "ftp"
   char filename[1020]; // Buffer for the file name

   // Prompt user for input
   printf("Enter command and file name (e.g., 'ftp filename.type'): ");
   if (fgets(input, sizeof(input), stdin) != NULL){
      // Parse input - expecting the format "ftp <file name>"
      if ((sscanf(input, "%3s %1019s", command, filename) == 2) && (strcmp(command, "ftp") == 0)){
         // blank if correct
      }else{
         printf("Incorrect format. Please use the format 'ftp <file name>'.\n");
      }
    }else{
        printf("Error input size. Try again.\n");
    }

   /* Create a UDP socket */
   int socket_FD = socket(AF_INET, SOCK_DGRAM, 0);
   // corner case
   if (socket_FD < 0){
      perror("socket creation failed");
      exit(EXIT_FAILURE);
   }

   /* Check existance of the file */
   if (access(filename, F_OK) == 0){ // F_OK as a flag
      printf("File '%s' exists. Waiting to send... \n", filename);
      const char *msg = "ftp";
      sendto(socket_FD, msg, strlen(msg), 0, (const struct sockaddr *) &server_addr, sizeof(server_addr));
   }else{
      printf("File does not exist. Transmission failed. \n");
      exit(EXIT_FAILURE);
   }

/* -------------------------------------------------------------------------- */
    /* Receive messages from clients */
   char buffer[1024];
   struct sockaddr_storage client_addr;
   socklen_t len = sizeof(client_addr);

   // waiting for seceive
   int n = recvfrom(socket_FD, (char *)buffer, 1024, 0, (struct sockaddr *) &client_addr, &len); // receive msg from a socket
   buffer[n] = '\0'; // Null-terminate the string

   // Check the message and respond
   const char *msg;
   if (strcmp(buffer, "yes") == 0) {
      msg = "A file transfer can start. \n";
      printf(msg);
   } else {
      msg = "Failed. \n";
      printf(msg);
      exit(EXIT_FAILURE); // break to send another msg
   }

   // Close the socket
   close(socket_FD);

   return 0;
}


