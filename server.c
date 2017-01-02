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

#define PORT 5050

#define BACKLOG 10 

void sigchld_handler(int s) { 
	// save errno in case of overwrite
	int saved_errno = errno;
	// -1 match any child process id
	// status : NULL ( if not NULL stores info in int * )
	// WNOHANG - return immediately if no child has termed
	while(waitpid(-1, NULL, WNOHANG) > 0);
	errno = saved_errno;
}

int socky() { 
	
	// Define desired socket type
	struct addrinfo hints
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; // Wildcard IP Address
	hints.ai_protocol = 0; // Any protocol

	int rv; // Return holder 
	struct addrinfo *servinfo;
	// Return a list of address structures from local
	// node set to NULL for loopback address 
	// service is the PORT number : 5050
	if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv)); 
		return 1;
	}

	// Try each address until we bind() successfully
	// Temp addrinfo struct for holding results
	struct addrinfo *p;
	int sockfd, yes=1;
	// Create the socket
	for (p = servinfo; p != NULL; p = p->ai_next) { 
		if ((sockfd = socket( p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) { 
			perror("server: socket");
			continue;
		}

	// Set the socket options 
	// SOL_SOCKET : This socket 
	// SO_REUSEADDR : Allow other process to bind this port #		
		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1 ) { 
			perror("setsockopt");
			exit(1);
		}

	// bind to the socket
	// the ai_addr is a pointer of struct type sockaddr
		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) { 
			close(sockfd);
			perror("server: bind");
			continue;
		}

	// If get to here = success break out the loop
		break;
	}

	// free up the servinfo structure
	freeaddrinfo(servinfo);

	if (p == NULL) { 
		fprintf(stderr, "server: failed to bind\n");
		exit(1);
	}

	// Listen for data on the sockfd, upto backlog
	if (listen(sockfd, BACKLOG) == -1) { 
		perror("listen");
		exit(1);
	}
	
	// Create a signal handler for children
	

}
