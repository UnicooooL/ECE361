/* Include packages */
#include "sec2.h"

/* Main function */
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
   // if fail
   if(print_to_net != 1){ 
      fprintf(stderr, "network form fail\n");
         exit(EXIT_FAILURE);
   }

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
   if (access(filename, F_OK) != 0){ // F_OK as a flag
      printf("File does not exist. Transmission failed. \n");
      exit(EXIT_FAILURE);
   }

   /* Open file */
    FILE *file = fopen(filename, "rb");
    if (file == NULL) {
        perror("Cannot open file. \n");
        exit(EXIT_FAILURE);
    }

   /* Find file size */
    fseek(file, 0L, SEEK_END);
    long file_size = ftell(file);
    rewind(file);

	/* Calculate total fragments */
    unsigned int total_fragments = (file_size / MAX_DATA_SIZE) + (file_size % MAX_DATA_SIZE != 0); // e.g. 2100/1000=2.1; so 0.1 will be at the new packet

	struct packet pkt;
    pkt.filename = (char*)filename;

 // ----------------------------------------------------------------------------------------------------  

	/* create extra buffer */
    char buffer[sizeof(struct packet) + 255]; // Buffer size = packet size + max filename length
    size_t buffer_pos = 0;
    size_t bytes_to_send;

	/* To send pkt and receive ACK */
    for (unsigned int frag_no = 0; frag_no < total_fragments; ++frag_no) {
        pkt.total_frag = total_fragments;
        pkt.frag_no = frag_no + 1;
        pkt.size = fread(pkt.filedata, 1, MAX_DATA_SIZE, file);

        // Reset buffer position for each packet
        buffer_pos = 0;

        // Serialize struct to buffer
        buffer_pos += snprintf(buffer + buffer_pos, sizeof(buffer) - buffer_pos, "%d:", pkt.total_frag);
        buffer_pos += snprintf(buffer + buffer_pos, sizeof(buffer) - buffer_pos, "%d:", pkt.frag_no);
        buffer_pos += snprintf(buffer + buffer_pos, sizeof(buffer) - buffer_pos, "%d:", pkt.size);
        buffer_pos += snprintf(buffer + buffer_pos, sizeof(buffer) - buffer_pos, "%s:", pkt.filename);
        memcpy(buffer + buffer_pos, pkt.filedata, pkt.size);
        buffer_pos += pkt.size;

        // Now buffer_pos is the total number of bytes to send
        bytes_to_send = buffer_pos;

        // Send packet
        if (sendto(sockfd, &pkt, sizeof(pkt), 0, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) {
            perror("Transmission failed. \n");
            exit(EXIT_FAILURE);
        }

        // Wait for ACK
		struct packet ack_pkt;
		int ack_len = recvfrom(sockfd, &ack_pkt, sizeof(ack_pkt), 0, NULL, NULL);

		// Check if there was an error receiving the packet
		if (ack_len < 0) {
			perror("Error receiving ACK/NACK");
			// Handle the error, possibly by resending the data packet
		} else if (ack_len != sizeof(ack_pkt)) {
			fprintf(stderr, "Received an incomplete ACK/NACK packet\n");
			// Handle the incomplete packet, possibly by resending the data packet
		} else {
			// Successfully received an ACK/NACK packet, check its contents
			if (ack_pkt.filedata == "ACK" && ack_pkt.frag_no == pkt.frag_no) {
				// Received the expected ACK, can send the next packet
			} else if (ack_pkt.filedata == "NACK") {
				// Received a NACK, must resend the current packet
				fprintf(stderr, "Received NACK for fragment %u\n", pkt.frag_no);
				// Resend
				frag_no = frag_no - 1;
			} else {
				// Received an ACK for a different fragment, this could be an error or an out-of-order ACK
				fprintf(stderr, "Received ACK for fragment %u, but expected %u\n", ack_pkt.ack_no, pkt.frag_no);
				// Handle the unexpected ACK by resending the data packet
				frag_no = frag_no - 1;
			}
		}

    // Close file and socket
    fclose(file);
    close(sockfd);

    return 0;
}