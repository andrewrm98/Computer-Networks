/* Andrew Morrison
 * Project 1
 * HTTP Server */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <signal.h>
#include <fcntl.h>

#define BACKLOG 10
#define MAXDATASIZE 4096
volatile sig_atomic_t terminate = 0;

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void sigchld_handler(int s)
{
    // waitpid() might overwrite errno, so we save and restore it:
    int saved_errno = errno;

    while(waitpid(-1, NULL, WNOHANG) > 0);

    errno = saved_errno;
}

/* exit cleanly */
void term(int sig) {
	terminate = 1;
}

int main(int argc, char * argv[]) {

	/* vatriables */
	int sockfd;
	int newfd;
	char * port;
	struct addrinfo hints;
	struct addrinfo *info, *p;
	struct sockaddr_storage their_addr;
	socklen_t sin_size;
	struct sigaction sa;
	int yes = 1;
	char s[INET6_ADDRSTRLEN];
	char buf[MAXDATASIZE];
	int rv;
	int numbytes;
	char message[1000];

	/* variables for sigterm */
	struct sigaction action;
	memset(&action, 0, sizeof(struct sigaction));
	action.sa_handler = term;
	sigaction(SIGTERM, &action, NULL);

	/* process input */
	if(argc != 2) {
		fprintf(stderr, "Usage: portNumber");
	} else {
		port = argv[1]; // get the specified port number
	}

	/* set up hints */
	memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    /* getaddrinfo */
    if ((rv = getaddrinfo(NULL, port, &hints, &info)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    /* bind */
    for(p = info; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("server: socket");
            continue;
        }

        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("server: bind");
            continue;
        }

        break;
    }
    freeaddrinfo(info); // free memory

     if (p == NULL)  {
        fprintf(stderr, "Error: bind\n");
        exit(1);
    }

    if (listen(sockfd, BACKLOG) == -1) {
        perror("Error: listen");
        exit(1);
    }

    /* reap dead processes */
    sa.sa_handler = sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;

    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }

    printf("Listening...\n");

     while(!terminate) {  // main accept() loop
        sin_size = sizeof their_addr;

        /* accept connection */
        newfd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);

        if (newfd == -1) {
            perror("accept");
            continue;
        }

        inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), s, sizeof s);
        printf("server: connected to %s\n", s);

        if (!fork()) { // this is the child process
            close(sockfd); // child doesn't need the listener
    		int count = 0;

            memset(buf, 0, MAXDATASIZE);
            recv(newfd, buf, MAXDATASIZE-1, 0);
            printf("\nBuffer: %s\n", buf);

			char * check = strstr(buf, "TMDG.html");
            char * check2 = strstr(buf, "index.html");

			/* 404 not found */
			if(!check && !check2) {
                printf("File not found\n");
				sprintf(message, "HTTP/1.1 404 Not Found\r\nContent-type: text/html\r\nContent-length: 0\r\n\r\n");
				send(newfd, message, strlen(message), 0);
                close(newfd);
                exit(0);
			} else {
                printf("File found\n");

				/* 200 OK */
            	/* Read html file */
				FILE * sendFile = fopen("TMDG.html", "r");
				fseek(sendFile, 0L, SEEK_END);
				int content_size = ftell(sendFile);
                char buffer[content_size];
				rewind(sendFile);
				sprintf(message, "HTTP/1.1 200 OK\r\nContent-type: text/html\r\nContent-length: %d\r\n\r\n", content_size);
				send(newfd, message, strlen(message), 0);

                int read = fread(buffer, sizeof(unsigned char), content_size, sendFile);
                char * buffer_ptr = buffer;
                if(read > 0) {
                    int numsent = send(newfd, buffer_ptr, (int)read, 0);
                    if(numsent < 1) {
                        fprintf(stderr, "Server: send");
                        break;
                    }
                }
                fclose(sendFile);
                close(newfd);
                exit(0);

			}

            close(newfd);
        }
    }

    printf("Process terminated\n");
    return 0;
}


