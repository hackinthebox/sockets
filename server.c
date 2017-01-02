#include <stdio.h> // standard c input output functions
#include <stdlib.h> // exit  
#include <unistd.h> // close and fork functions , NULL pointer 
#include <errno.h> // perror handling 
#include <string.h> // string functions 
#include <netdb.h> // addrinfo types 
#include <sys/socket.h> // sockaddr structure
#include <arpa/inet.h> // inet_ntop ( converting client address to string )
#include <sys/wait.h> // waitpid constants ( WNOHANG ) 
#include <signal.h> // signal handling 

#define PORT "5050"

#define BACKLOG 10 

// Function for reaping zombie processes
void sigchld_handler(int s) { 
	// save errno in case of overwrite
	int saved_errno = errno;
	// -1 match any child process id
	// status : NULL ( if not NULL stores info in int * )
	// WNOHANG - return immediately if no child has termed
	while(waitpid(-1, NULL, WNOHANG) > 0);
	errno = saved_errno;
}


// Function for reading the client's address 
void *get_in_addr(struct sockaddr *sa) { 
	if (sa->sa_family == AF_INET) { 
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}
	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}


int socky() { 
	
	// Define desired socket type
	struct addrinfo hints;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
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
	
	// Create a signal handler for child procs
	// create a structure of type sigaction, this will hold a pointer to the handler function and behaviour flags
	struct sigaction sa;
	sa.sa_handler = sigchld_handler; // set the handler function , that will reap dead procs
	sigemptyset(&sa.sa_mask);
	// Specify the behaviour flags to use with the signal handler 
	// SA_RESTART - 
	sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;
	// sigaction function is used to examine and change the signal action
	// we specify the SIGCHLD signal and the sigaction structure with our defined handler function
	if (sigaction(SIGCHLD, &sa, NULL) == -1) { 
		perror("sigaction");
		exit(1);
	}
	
	printf("server: waiting for connections...\n");
	
	// loop to accept incomming connections
	// accept on the socket descriptor, specify a struct for the client's address and sizeof address
	
	// new socket descriptor for holding client socket information
	int new_fd, sin_size;
	// sockaddr struct 
	struct sockaddr their_addr;
	char s[INET6_ADDRSTRLEN];
	while(1) { // main accept() loop, forks children to handle data 
		sin_size = sizeof their_addr;
		// accept their_addr will hold the client information if we don't want this data these could be set to NULL
		// new_fd will hold the socket descriptor associated with the client
		// the original sockfd is not assocaited with the socket and is available for new connections
		new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
		if (new_fd == -1) {
			perror("accept");
			continue;
		}
	
	// inet_ntop will convert a binary IPv4/6 address to text.
	// the char buffer s will hold the text string of the client address, this is then printed.
	inet_ntop(their_addr.sa_family, get_in_addr((struct sockaddr *)&their_addr), s, sizeof(s));
	printf("server: got connection from %s\n", s);

	if (!fork()) { // this is the child process ( same as : if (fork() == 0 )
		close(sockfd);
		if (send(new_fd, "Hello, world!\n", 15, 0) == -1){
			perror("send");
		}
		close(new_fd);
		exit(0);
	}
	close(new_fd); // parent doesn't need the socket descriptor
	}
	return 0;
}		

int main() { 
	socky();
}
