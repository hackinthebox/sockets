#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define PORT "5050"
#define MAXDATASIZE 100
#define MAXADDRSIZE 50

// function to get sockaddr ( whether IPv4 or 6 ) 
void *get_in_addr(struct sockaddr *sa) {
	if (sa->sa_family == AF_INET) { 
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}
	
	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

// Function to return socket descriptor
int socky(char *a) {
	struct addrinfo hints, *servinfo, *p;
	
	// Define hints struct for socket type 	
	memset(&hints,0,sizeof hints);
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	int rv;
	// Get the address structs from the server
	if ((rv = getaddrinfo(a, PORT, &hints, &servinfo)) != 0){ 
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return(1);
	}

	// loop through the address and connect to the first we can
	int sockfd;
	for(p = servinfo; p != NULL; p = p->ai_next) {
		// create the local socket and get socket descriptor
		if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
			perror("client: socket");
			continue;
		}
		// connect to the server addr struct
		if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("client: connect");
			continue;
		}
		break;
	}
	if (p == NULL) { 
		fprintf(stderr, "client: failed to connect\n");
		return(2);
	}else{
	char s[INET6_ADDRSTRLEN];
	inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr), s, sizeof(s));
	printf("client: connecting to %s\n", s);
	return(sockfd);
	}
}

// Function to recieve data from the socket up to max data size
int get_data(int sockfd, char *buf){
	int numbytes;
	if (( numbytes = recv(sockfd, buf, (MAXDATASIZE-1), 0)) == -1) {
		perror("recv");
		exit(1);
	}
	buf[numbytes]= '\0';
	return numbytes;
}

int main(int argc, char **argv) {
	if (argc != 2) { 
		fprintf(stderr,"usage: client <hostname>\n");
		exit(1);
	}
	// open socket and connect to the server
	int sockfd = socky(argv[1]);
	char buf[MAXDATASIZE];
	int bytes_recv = get_data(sockfd, buf);
	
	printf("client: recieved '%s'\n",buf);
	
	close(sockfd);
	
	return(0);
}
