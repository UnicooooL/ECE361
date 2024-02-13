# CC=gcc

# all: server deliver
# server: server.o
#     # $(CC) server.c -o server
# deliver: deliver.o
#     # $(CC) client.c -o client
# clean:
# 	rm -f *.o server deliver
#     # UnicooooL/ECE361/Makefile

TARGET = server deliver

all: ${TARGET}
server: server.c sec2.h
	gcc -g server.c -o server
deliver: deliver.c sec2.h
	gcc -g deliver.c -o deliver
clean:  
	rm -f ${TARGET}
   