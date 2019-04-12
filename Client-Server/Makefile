# Andrew Morrison
# Client and Server Makefile for project 1
# Networks

CC = gcc
CFLAGS = -Wall
OBJS = client.o
OBJS2 = server.o 

all: client server

client: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o client

client.o: client.c 
	$(CC) $(CFLAGS) -c client.c

server: $(OBJS2)
	$(CC) $(CFLAGS) $(OBJS2) -o server

server.o: server.c 
	$(CC) $(CFLAGS) -c server.c

clean:
	rm -f *~ *.o client server