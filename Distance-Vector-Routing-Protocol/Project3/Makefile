# CS Networks Project 3 Makefile
# Andrew Morrison
# armorrison@wpi.edu

CC = gcc
CFLAGS = -g
OBJS = node0.o node1.o node2.o node3.o project3.o

all: project3

project3: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o project3

node0.o: node0.c
	$(CC) $(CFLAGS) -c node0.c

node1.o: node1.c
	$(CC) $(CFLAGS) -c node1.c

node2.o: node2.c
	$(CC) $(CFLAGS) -c node2.c

node3.o: node3.c
	$(CC) $(CFLAGS) -c node3.c

project3.o: project3.c
	$(CC) $(CFLAGS) -c project3.c

clean:
	rm -f *~ *.o project3 node0 node1 node2 node3