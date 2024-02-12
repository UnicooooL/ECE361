#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define BUFFER_SIZE 1100
#define DATA_SIZE 1000

struct packet{
    unsigned int total_frag;
    unsigned int frag_no;
    unsigned int size;
    char filename[100];
    char filedata[1000];
};

void serialize_packet(struct packet *pkt, char* buffer){
    sprintf(buffer, "%u:%u:%u:%s:", pkt->total_frag, pkt->frag_no, pkt->size, pkt->filename);
    memcpy(buffer + strlen(buffer), pkt->filedata, pkt->size);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Please enter sever IP address and Port number\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int sockfd;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];
    char file_name[100];
    char command[4];

    // Create a UDP socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Set server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[2]));
    server_addr.sin_addr.s_addr = inet_addr(argv[1]);

    // Ask for input
    printf("Enter the command and file name: ");
    
    scanf("%3s %s",command, file_name);
    //perror("Checkpoint 0");

    if(strcmp(command, "ftp") != 0){
        printf("Invalid command. Please start with 'ftp'. \n");
        return 1;
    }

    // Check if file exists
    //perror("Checkpoint 1");
    FILE *file = fopen(file_name, "r");
    //perror("Checkpoint 2");
    if(!file){
        perror("File openning failed");
        exit(1);
    }
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    unsigned int total_frags = (file_size / DATA_SIZE) + (file_size % DATA_SIZE != 0);
    struct packet pkt;
    unsigned int frag_num = 0;

    //perror("Checkpoint 3");

    while(!feof(file)){
        //perror("Checkpoint 4");
        memset(&pkt, 0, sizeof(struct packet));
        //perror("Checkpoint 4.1");
        pkt.total_frag = total_frags;
        pkt.frag_no = ++frag_num;
        pkt.size = fread(pkt.filedata,1, DATA_SIZE, file);
        //perror("Checkpoint 4.2");
        strcpy(pkt.filename, file_name);
        //perror("Checkpoint 4.3");

        serialize_packet(&pkt, buffer); //stores header seperated by column + file data in buffer
        //perror("Checkpoint 5");

        if(sendto(sockfd, buffer, pkt.size + strlen(buffer), 0, (struct sockaddr *) &server_addr, sizeof(server_addr))<0){
            perror("Error in deliver.c send to");
            exit(1);
        }
        //perror("Checkpoint 6");

        memset(buffer, 0, BUFFER_SIZE);
        if(recvfrom(sockfd, buffer, BUFFER_SIZE, 0, NULL, NULL) < 0){
            perror("Error in deliver.c recvfrom");
            exit(1);
        }
        //perror("Checkpoint 7");

        printf("Received ACK for fragment: %u\n", frag_num);
        //perror("Checkpoint 8");
    }

    fclose(file);
    close(sockfd);
    return 0;
} 
