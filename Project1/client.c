/* Andrew Morrison
	Networks
	Project 1
	HTTP Client */

#include <arpa/inet.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <fcntl.h>

#define MAXDATASIZE 100 // max number of bytes we can get at once 

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char* argv[]) {
	/* Necessary variables */
	int status;					// check for error
	struct addrinfo hints;		// necessary info
	struct addrinfo *servinfo;	// points to the results
	struct addrinfo *p;
	int sockfd, numbytes;		// socket file descriptor
	int rTTFlag = 0;			// 0 for no, 1 for yes
	struct timeval tv;
	time_t startTime, endTime, totalTime;
	char * node;
	const char * service;
	char s[INET6_ADDRSTRLEN];
	char buf[MAXDATASIZE];
	char message[100];
	int len;
	int count = 0;
	char * pch;

	/* for tokenizing */
	char * path = malloc(100);
	char * address;
	//char * token;
	char tokenA[50];
	char tokenB[50];

	/* Check argument length */
	if(argc < 2 || argc > 4) {
		fprintf(stderr, "Usage: options, server url, port number");
		exit(1);
	}

	/* get the node and service variables.. slightly different depending on whether an option is provided or not */
	if(argc == 4) {
		node = argv[2];
		//printf("node set\n");
		service = argv[3];

		if(strcmp(argv[1], "-p") == 0) // print the RTT
			rTTFlag = 1;
		else {
			fprintf(stderr, "Error. Option not recognized.");
			exit(1);
		}

	} else if (argc == 3) {
		node = argv[1];
		service = argv[2];
	}

	//printf("before tokenize\n");
	//printf("Node: %s\n", node);
	pch = strtok(node, "/");
	address = pch;

	while(pch != NULL) {
	
		if (count != 0) {
			strcpy(tokenA, "/");
			strcpy(tokenB, pch);
			strcat(path , strcat(tokenA, tokenB));
			//printf("Hello\n");
		}
		count++;
		pch = strtok(NULL, "/");
	}

	strcpy(tokenA, "/");
	strcat(path, tokenA);

	/* print out the path and address */
	printf("Address: %s\n", address);
	printf("Path: %s\n", path);

	/* set up GET message */
	if (strcmp(path, "") == 0) {
		sprintf(message, "GET / HTTP/1.1\r\nHost: %s\r\n\r\n", node);
		len = strlen(message);
	} else {
		sprintf(message, "GET %s HTTP/1.1\r\nHost: %s\r\n\r\n", path, address);
		len = strlen(message);
	}

	/* Set up hints */
	memset(&hints, 0, sizeof(hints));	// set hints to all 0s
	hints.ai_family = AF_UNSPEC;		// IPv4 or IPv6
	hints.ai_socktype = SOCK_STREAM;	// TCP socket

	/* getaddrinfo */
	if((status = getaddrinfo(node, service, &hints, &servinfo)) != 0) {
		fprintf(stderr, "error");
		return 1;
	}

	/* get the start time for RTT */
	gettimeofday(&tv, NULL);
	startTime = tv.tv_sec * 1000 + (tv.tv_usec) / 1000;

	/* Loop until we can connect */
	for(p = servinfo; p != NULL; p = p->ai_next) {

		/* socket */
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("client: socket");
            continue;
        }

        /* connect */
        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("client: connect");
            continue;
        }

        /* get end time for RTT */
        gettimeofday(&tv, NULL);
        endTime = tv.tv_sec * 1000 + (tv.tv_usec) / 1000;
        totalTime = endTime - startTime;

        if(rTTFlag == 1) {
        	printf("\nRound trip time: %ld milliseconds\n", totalTime);
        }

        break;
    }

    //printf("Made it past connect\n");

    if(p == NULL) {
    	fprintf(stderr, "Client failed to connect.");
    	return 2;
    }

    /* Convert IP address to text */
    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr), s, sizeof(s));
    printf("Socket : %d\n", sockfd);
    printf("client: connecting to %s\n", s);

    freeaddrinfo(servinfo); // free memory

    /* send message */
    printf("Message: %s\n", message);

    if(len != (send(sockfd, message, strlen(message), 0))) {
    	perror("client: send");
    	exit(1);
    }
    printf("Message sent.\n");
    printf("Client recieved: '\n");

    /* set the socket to non blocking
    This prevents the socket from blocking us while waiting for more data, which would cause the program to hang */
    fcntl(sockfd, F_SETFL, O_NONBLOCK);

    // count will allow us to check if we have waited for data multiple times, and break if we have
    int c = 0;

    while(1) {
    	//printf("Made it to memset\n");
    	memset(buf, 0, MAXDATASIZE);// reset buf
    	if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) < 0) {
        	sleep(1);				// sleep for 1 second
        	c++;
        	if(c > 2)			// if we waited more than twice in a row, there is probably no more data to recieve
        		break;
    	}
        else {
        	printf("%s\n", buf);
        	c = 0; 				// reset count since we just recieved data
        }
	}

    close(sockfd);
    return 0;
}



