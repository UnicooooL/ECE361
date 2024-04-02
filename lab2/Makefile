

TARGET = server client
LDFLAGS=-pthread

all: ${TARGET}
server: server.c client.h
	gcc -g $(LDFLAGS) server.c -o server
client: client.c server.h
	gcc -g $(LDFLAGS) client.c -o client
clean:  
	rm -f ${TARGET}
   