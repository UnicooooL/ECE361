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
    char buffer[1024];
	struct packet rec_pkt;
    struct sockaddr_storage client_addr;
    socklen_t len = sizeof(client_addr);

    while (1) {
        int n = recvfrom(socket_FD, &rec_pkt, sizeof(rec_pkt), 0, (struct sockaddr *) &client_addr, &len); // receive msg from a socket
        buffer[n] = '\0'; // Null-terminate the string

        // Check the message and respond
        const char *msg;
        if (strcmp(buffer, "ftp") == 0) {
            msg = "yes";
        } else {
            msg = "no";
        }

		// create ack pkt
		struct packet send_pkt = rec_pkt;
		if (n > 0) { // received
			send_pkt.filedata = "ACK"; // indicate ack send
			FILE *rec_file = fopen(rec_pkt.filename, "wb"); // Open the file in binary write mode
			if (file == NULL) {
				perror("Failed to open file. \n");
				exit(EXIT_FAILURE);
			}
			size_t bytes_written = fwrite(rec_pkt.filedata, 1, rec_pkt.size, rec_file);
			if (bytes_written != rec_pkt.size) {
				perror("Failed to write the complete data. \n");
				// Handle the partial write error
				exit(EXIT_FAILURE);
			}
			fclose(rec_file); // Close the file


		}else {
			send_pkt.filedata = "NACK";
		}
		sendto(socket_FD, &send_pkt, sizeof(send_pkt), 0, (const struct sockaddr *) &client_addr, len);

		// if fully received
		if (rec_pkt.frag_no == rec_pkt.total_frag) {
			break;
		}
    }

    // Close the socket
    close(socket_FD);

    return 0;
}

