#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netdb.h>
#define _OPEN_SYS_SOCK_IPV6
#include <arpa/inet.h>
#include <time.h>
#include <regex.h>
#include <sys/types.h>
#include <ctype.h>
#include <errno.h>
#include <stdbool.h>

/* Defined the maximum segment size */
#define MAX_DATA_SIZE 1000
#define BUFFER 1100

/* Given structure for pocket */
typedef struct packet {
    unsigned int total_frag;
    unsigned int frag_no;
    unsigned int size;
    char filename[BUFFER];
    char filedata[MAX_DATA_SIZE];
}packet;

/* User defined */
void packetToString(const struct packet *packet, void *result) {
    
    // Initialize string buffer
    memset(result, 0, BUFFER);

    // Load data into string
    int cursor = 0;
    sprintf(result, "%d", packet -> total_frag);
    cursor = strlen(result);
    memcpy(result + cursor, ":", sizeof(char));
    ++cursor;
    
    sprintf(result + cursor, "%d", packet -> frag_no);
    cursor = strlen(result);
    memcpy(result + cursor, ":", sizeof(char));
    ++cursor;

    sprintf(result + cursor, "%d", packet -> size);
    cursor = strlen(result);
    memcpy(result + cursor, ":", sizeof(char));
    ++cursor;

    sprintf(result + cursor, "%s", packet -> filename);
    cursor += strlen(packet -> filename);
    memcpy(result + cursor, ":", sizeof(char));
    ++cursor;

    memcpy(result + cursor, packet -> filedata, sizeof(char) * MAX_DATA_SIZE);

    // printf("%s\n", packet . filedata);
    // printf("Length of data: %d\n", strlen(packet . filedata));

}

void stringToPacket(const void* str, struct packet *packet) {

    // Compile Regex to match ":"
    regex_t regex;
    if(regcomp(&regex, "[:]", REG_EXTENDED)) {
        fprintf(stderr, "Could not compile regex\n");
    }

    // Match regex to find ":" 
    regmatch_t pmatch[1];
    int cursor = 0;
    char buf[BUFFER];

    // Match total_frag
    if(regexec(&regex, str + cursor, 1, pmatch, REG_NOTBOL)) {
        fprintf(stderr, "Error matching regex\n");
        exit(1);
    }
    memset(buf, 0, BUFFER * sizeof(char));
    memcpy(buf, str + cursor, pmatch[0].rm_so);
    packet -> total_frag = atoi(buf);
    cursor += (pmatch[0].rm_so + 1);

    // Match frag_no
    if(regexec(&regex, str + cursor, 1, pmatch, REG_NOTBOL)) {
        fprintf(stderr, "Error matching regex\n");
        exit(1);
    }
    memset(buf, 0,  BUFFER * sizeof(char));
    memcpy(buf, str + cursor, pmatch[0].rm_so);
    packet -> frag_no = atoi(buf);
    cursor += (pmatch[0].rm_so + 1);

    // Match size
    if(regexec(&regex, str + cursor, 1, pmatch, REG_NOTBOL)) {
        fprintf(stderr, "Error matching regex\n");
        exit(1);
    }
    memset(buf, 0, BUFFER * sizeof(char));
    memcpy(buf, str + cursor, pmatch[0].rm_so);
    packet -> size = atoi(buf);
    cursor += (pmatch[0].rm_so + 1);

    // Match filename
    if(regexec(&regex, str + cursor, 1, pmatch, REG_NOTBOL)) {
        fprintf(stderr, "Error matching regex\n");
        exit(1);
    }

    //memset(packet -> filename, 0, BUFFER * sizeof(char));
    memcpy(packet -> filename, str + cursor, pmatch[0].rm_so);

    packet -> filename[pmatch[0].rm_so] = 0;
    cursor += (pmatch[0].rm_so + 1);
    
    // Match filedata
    //printf("\n\n\n%s\n\n\n", packet -> filedata);
    //memset(packet -> filedata, 0, MAX_DATA_SIZE * sizeof(char));
    memcpy(packet -> filedata, str + cursor, packet -> size);
    //printf("\n\n\n%s\n\n\n", str + cursor);

    // printf("total_frag:\t%d\n", packet -> total_frag);
    // printf("frag_no:\t%d\n", packet -> frag_no);
    // printf("size:\t%d\n", packet -> size);
    // printf("filename:\t%s\n", packet -> filename);
    //printf("filedata:\t%s\n", packet -> filedata);

}

// void printPacket(struct packet *packet) {
//     printf("total_frag = %d,\n frag_no = %d, size = %d, filename = %s\n", packet -> total_frag, packet -> frag_no, packet -> size, packet -> filename);
//     char data[MAX_DATA_SIZE + 1] = {0};
//     memcpy(data, packet -> filedata, MAX_DATA_SIZE);
//     printf("%s", data);
// }
