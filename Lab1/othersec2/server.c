#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdbool.h>
#include "packet.h"

int main(int argc, char const *argv[])
{
    if (argc != 2) { 
        printf("usage: server -<UDP listen port>\n");
        exit(0);
    }
    int port = atoi(argv[1]);

    int sockfd;	
    // open socket (DGRAM)
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
        printf("socket error\n");
        exit(1);
    }

    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);	
    memset(serv_addr.sin_zero, 0, sizeof(serv_addr.sin_zero));

    // bind to socket
    if ((bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr))) == -1) {
        printf("bind error\n");
        exit(1);
    }

    char buf[BUF_SIZE] = {0};
    struct sockaddr_in cli_addr; 
    socklen_t clilen; // length of client info
    // recvfrom the client and store info in cli_addr so as to send back later
    if (recvfrom(sockfd, buf, BUF_SIZE, 0, (struct sockaddr *) &cli_addr, &clilen) == -1) {
        printf("recvfrom error\n");
        exit(1);
    }

    // send message back to client based on message recevied
    if (strcmp(buf, "ftp") == 0) {
        if ((sendto(sockfd, "yes", strlen("yes"), 0, (struct sockaddr *) &cli_addr, sizeof(cli_addr))) == -1) {
            printf("sendto error\n");
            exit(1);
        }
    } else {
        if ((sendto(sockfd, "no", strlen("no"), 0, (struct sockaddr *) &cli_addr, sizeof(cli_addr))) == -1) {
            printf("sendto error\n");
            exit(1);
        }
    }















    /* defination */
    Packet packet; // define packet
    packet.filename = (char *) malloc(BUF_SIZE); // define filename's size
    char filename[BUF_SIZE] = {0}; // local file name
    FILE *pFile = NULL; // empty file
    bool *fragRecv = NULL; // determine the frag number






    for (;;) {

        if (recvfrom(sockfd, buf, BUF_SIZE, 0, (struct sockaddr *) &cli_addr, &clilen) == -1) { // buf size = max size; buf store pocket in string
            printf("recvfrom error\n"); // not receive
            exit(1);
        }

        stringToPacket(buf, &packet); // recover to packet

        if (!pFile) { // if the file create or not
            strcpy(filename, packet.filename); // copy to local file name

            while (access(filename, F_OK) == 0) { // check for existance
                char *pSuffix = strrchr(filename, '.'); // store the location of '.' for local file name
                char suffix[BUF_SIZE + 1] = {0}; // define a string to store the extension
                strncpy(suffix, pSuffix, BUF_SIZE - 1); // store all data from '.' to the end of size into local storage
                *pSuffix = '\0'; // original position of '.' is equal to termination char
                strcat(filename, " copy"); // add 'copy' before the local filename extension
                strcat(filename, suffix); // add the extension to local file name
            } 
            pFile = fopen(filename, "w"); // create the file
        }







        if (!fragRecv) { // if this is th first segment
            fragRecv = (bool *) malloc(packet.total_frag * sizeof(fragRecv)); // boolean array to check for received frag or not
            for (int i = 0; i < packet.total_frag; i++) {
                fragRecv[i] = false; // initialization to false
            }
        }

        if (!fragRecv[packet.frag_no]) { // if not wrote before	
            int numbyte = fwrite(packet.filedata, sizeof(char), packet.size, pFile); // write the data into the copied func
            if (numbyte != packet.size) { // if not fully received
                printf("fwrite error\n"); // need sth to triger delete partial writing
                exit(1);
            } 
            fragRecv[packet.frag_no] = true; // flip the notation
        }
        strcpy(packet.filedata, "ACK"); // rewrite file data of received packet (not the local one)

        packetToString(&packet, buf); // turn packet into string for transmission

        /*send ack doc*/
        if ((sendto(sockfd, buf, BUF_SIZE, 0, (struct sockaddr *) &cli_addr, sizeof(cli_addr))) == -1) {
            printf("sendto error\n");
            exit(1);
        }
        // break for loop if transmission finished
        if (packet.frag_no == packet.total_frag) {
            printf("File %s transfer completed\n", filename);
            break;
        }
    }










    close(sockfd);
    fclose(pFile);
    free(fragRecv);
    free(packet.filename);
    return 0;
}
