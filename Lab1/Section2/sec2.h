#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netdb.h>
#define _OPEN_SYS_SOCK_IPV6
#include <arpa/inet.h>

/* Defined the maximum segment size */
#define MAX_DATA_SIZE 1000

/* Given structure for pocket */
struct packet {
    unsigned int total_frag;
    unsigned int frag_no;
    unsigned int size;
    char* filename;
    char filedata[MAX_DATA_SIZE];
};

