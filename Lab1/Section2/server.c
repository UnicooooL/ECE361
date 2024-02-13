/* Include packages */
#include "sec2.h"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <UDP listen port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

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
    char buffer[BUFFER] = {0};
	struct packet rec_pkt; // received pkt
    struct sockaddr_storage client_addr;
    socklen_t len = sizeof(client_addr);
    char filename[1024] = "copy";
    char* checking = NULL;
    FILE *rec_file = NULL;
    int rec_frag = 0;

    while (1) {
        int n = recvfrom(socket_FD, buffer, sizeof(buffer), 0, (struct sockaddr *) &client_addr, &len); // receive msg from a socket
        buffer[n] = '\0'; // Null-terminate the string
        //printf("%s\n", buffer);
        memset(rec_pkt.filename, 0, BUFFER);
        memset(rec_pkt.filedata, 0, MAX_DATA_SIZE);
        stringToPacket(buffer, &rec_pkt); // recover to packet
      
		// create ack pkt
		struct packet send_pkt;
        strcpy(send_pkt.filename, rec_pkt.filename);
        send_pkt.frag_no = rec_pkt.frag_no;
        char ack_info[BUFFER]; 
        memset(ack_info, 0, BUFFER);

		if (n > 0) { // received

            /* name the local copied file */
            if (access(checking, F_OK) != 0) { // if exist
                checking = strcat(filename, rec_pkt.filename); // copy+filename+extension+\0
                rec_file = fopen(filename, "w"); // Open the file in binary write mode
                if (rec_file == NULL) { // if not created correctly
                    perror("Failed to open file. \n");
                    exit(EXIT_FAILURE);
                }
            }
            
            /* copy file data */
            if (rec_frag + 1 == rec_pkt.frag_no) { // if curr frag not wrote
                int bytes_written = fwrite(rec_pkt.filedata, sizeof(char), rec_pkt.size, rec_file);
                //printf("%s\n", rec_pkt.filedata);
               
                if (bytes_written != rec_pkt.size) {
                    perror("Failed to write the complete data. \n");
                    // Handle the partial write error
                    exit(EXIT_FAILURE);
                }  
                /* send ack doc for curr frag */
                strcpy(send_pkt.filedata, "ACK\0"); // rewrite data
                send_pkt.size = sizeof(send_pkt.filedata);
                packetToString(&send_pkt, ack_info);
                int m = sendto(socket_FD, ack_info, sizeof(ack_info), 0, (const struct sockaddr *) &client_addr, len);
                if (m <= 0) { // not send
                    printf("Frag num %d ack doc transmission failed. \n", rec_frag+1);
                    exit(EXIT_FAILURE);
                }
                rec_frag++; // update frag number for tracking
            }
            // if fully received
            if (rec_pkt.frag_no == rec_pkt.total_frag) {
                printf("==================================== \nFile %s transmission finished! \n", filename);
            }

        // if not received file
		}else {
			printf("File receive failed, please try again, \n");
		}
        fclose(rec_file); // Close the file
    }

    /* Close the socket and file */
    close(socket_FD);

    return 0;
}