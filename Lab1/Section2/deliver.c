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
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Cannot open file. \n");
        exit(EXIT_FAILURE);
    }

   /* Find file size */
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    rewind(file);

	/* Calculate total fragments */
    unsigned int total_fragments = (file_size / MAX_DATA_SIZE) + (file_size % MAX_DATA_SIZE != 0); // e.g. 2100/1000=2.1; so 0.1 will be at the new packet
    printf("Number of fragments: %d\n", total_fragments);

 // ----------------------------------------------------------------------------------------------------  

	
    size_t buffer_pos = 0;
    size_t bytes_to_send;
    struct sockaddr_storage client_addr;
    

	/* To send pkt and receive ACK */
    for (unsigned int frag_no = 0; frag_no < total_fragments; ++frag_no) {
        
        //creat packet and update the info 
        //struct packet* pkt_mal = malloc(BUFFER);
        struct packet pkt;// = *pkt_mal;
        strcpy(pkt.filename, filename);
        //pkt.filename = (char*)filename;
        pkt.total_frag = total_fragments;
        pkt.frag_no = frag_no + 1;
        memset(pkt.filedata, 0, MAX_DATA_SIZE);
        fread((void*)pkt.filedata, 1, MAX_DATA_SIZE, file);
        if(frag_no < total_fragments){
            pkt.size = MAX_DATA_SIZE;
        }else{
            // This packet is the last packet
            pkt.size = (file_size -1) % MAX_DATA_SIZE + 1;
        }
        

        /* create extra buffer */
        char buffer[BUFFER]; // Buffer size = packet size + max filename length
        packetToString(&pkt, buffer);
        //printf("%s\n", buffer);

        /*
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
        bytes_to_send = buffer_pos;*/


        struct timeval timeout;
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;

        //============================================================
        // clock_t t_send, t_rec;  // timer variables
        // t_send = clock();
        // Send packet
        if (sendto(socket_FD, buffer, sizeof(buffer), 0, (const struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
            perror("Transmission failed. \n");
            exit(EXIT_FAILURE);
        }

        // Wait for ACK
        struct packet ack_pkt;
		char ack_buf[BUFFER];
        memset(ack_buf, 0, BUFFER);
        
        socklen_t len = sizeof(server_addr);
		int ack_len = recvfrom(socket_FD, ack_buf, sizeof(ack_buf), 0, (struct sockaddr *) &server_addr, &len);

		// Check if there was an error receiving the packet
		if (ack_len < 0) {
			perror("Error receiving ACK/NACK");
			// Handle the error, possibly by resending the data packet
		} else if (ack_len != sizeof(ack_buf)) {
			fprintf(stderr, "Received an incomplete ACK/NACK packet\n");
			// Handle the incomplete packet, possibly by resending the data packet
		} else {
            // t_rec = clock();
            // double time = (t_rec - t_send) / CLOCKS_PER_SEC;
            // fprintf(stdout, "RTT = %f sec.\n", time);  
        //=================================================================
            stringToPacket(ack_buf, &ack_pkt);
			// Successfully received an ACK/NACK packet, check its contents
			if (strcmp(ack_pkt.filedata, "ACK") == 0 && ack_pkt.frag_no == pkt.frag_no) {
				// Received the expected ACK, can send the next packet
			} else if(strcmp(ack_pkt.filedata, "ACK") != 0) {
				// Received a NACK, must resend the current packet
				fprintf(stderr, "Received ACK failed for fragment %u\n", pkt.frag_no);
				// Resend
				frag_no = frag_no - 1;
			} else {
				// Received an ACK for a different fragment, this could be an error or an out-of-order ACK
				fprintf(stderr, "Received ACK for fragment %u, but expected %u\n", ack_pkt.frag_no, pkt.frag_no);
				// Handle the unexpected ACK by resending the data packet
				frag_no = frag_no - 1;
			}
		}
        //free(pkt_mal);
    }
    // trans
    printf("==================================== \nFile %s transmission finished! \n", filename);

    // Close file and socket
    fclose(file);
    close(socket_FD);

    return 0;
}