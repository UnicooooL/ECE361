

TARGET = server client
LDFLAGS=-pthread

all: ${TARGET}
server: server.c
	gcc -g $(LDFLAGS) server.c -o server
client: client.c
	gcc -g $(LDFLAGS) client.c -o client
clean:  
	rm -f ${TARGET}
   