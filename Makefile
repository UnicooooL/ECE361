CC=gcc

all: server client
server: server.c
    # $(CC) server.c -o server
client: client.c
    # $(CC) client.c -o client
clean:
    # rm -f server client
    # UnicooooL/ECE361/Makefile