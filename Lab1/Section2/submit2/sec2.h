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
void PtoS(const struct packet *pkt, void *result) {
    sprintf(result, "%d:%d:%d:%s:", pkt->total_frag, pkt->frag_no, pkt->size, pkt->filename);
    memcpy(result + strlen(result), pkt->filedata, pkt->size);
}


void StoP(const void* str, struct packet *packet) {

    // Compile Regex to match ":"
    regex_t regex;
    if(regcomp(&regex, "[:]", REG_EXTENDED)) {
        fprintf(stderr, "Could not compile regex\n");
    }

    // Match regex to find ":" 
    regmatch_t pmatch[1];
    int target = 0;
    char buf[BUFFER];

    // Match total_frag
    if(regexec(&regex, str + target, 1, pmatch, REG_NOTBOL)) {
        fprintf(stderr, "Error matching regex\n");
        exit(1);
    }
    memset(buf, 0, BUFFER * sizeof(char));
    memcpy(buf, str + target, pmatch[0].rm_so);
    packet -> total_frag = atoi(buf);
    target += (pmatch[0].rm_so + 1);

    // Match frag_no
    if(regexec(&regex, str + target, 1, pmatch, REG_NOTBOL)) {
        fprintf(stderr, "Error matching regex\n");
        exit(1);
    }
    memset(buf, 0,  BUFFER * sizeof(char));
    memcpy(buf, str + target, pmatch[0].rm_so);
    packet -> frag_no = atoi(buf);
    target += (pmatch[0].rm_so + 1);

    // Match size
    if(regexec(&regex, str + target, 1, pmatch, REG_NOTBOL)) {
        fprintf(stderr, "Error matching regex\n");
        exit(1);
    }
    memset(buf, 0, BUFFER * sizeof(char));
    memcpy(buf, str + target, pmatch[0].rm_so);
    packet -> size = atoi(buf);
    target += (pmatch[0].rm_so + 1);

    // Match filename
    if(regexec(&regex, str + target, 1, pmatch, REG_NOTBOL)) {
        fprintf(stderr, "Error matching regex\n");
        exit(1);
    }

    //memset(packet -> filename, 0, BUFFER * sizeof(char));
    memcpy(packet -> filename, str + target, pmatch[0].rm_so);

    packet -> filename[pmatch[0].rm_so] = 0;
    target += (pmatch[0].rm_so + 1);
    
    // Match filedata
    memcpy(packet -> filedata, str + target, packet -> size);

}
