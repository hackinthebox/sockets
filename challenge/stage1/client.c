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

#define MAXDATASIZE 128
#define MAXADDRSIZE 50
#define SAVEDFILE "saved_file.txt"
#define TRUE 1

// function to get sockaddr ( whether IPv4 or 6 ) 
void *get_in_addr(struct sockaddr *sa) {
	if (sa->sa_family == AF_INET) { 
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}
	
	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

// Function to return socket descriptor
int socky(char *a, const char *port_num) {
	struct addrinfo hints, *servinfo, *p;
	
	// Define hints struct for socket type 	
	memset(&hints,0,sizeof hints);
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	int rv;
	// Get the address structs from the server
	if ((rv = getaddrinfo(a, port_num, &hints, &servinfo)) != 0){ 
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
	printf("[CLIENT] connecting to %s\n", s);
	return(sockfd);
	}
}

int receive_data(int sockfd, FILE *fp) {
	int size_recv, total_size=0;
	char buf[MAXDATASIZE];
	while (TRUE) {
	size_recv = recv(sockfd, buf, sizeof(buf), 0);
        if ((size_recv == -1)) {
                perror("recv");
                exit(1);
        } else if ((size_recv == 0)) {
		break;
	}
		fwrite(buf, 1, size_recv, fp);
		total_size = total_size + size_recv;
	}
	return total_size;
}


int main(int argc, char **argv) {
	if (argc != 3) {
		fprintf(stderr,"usage: client <hostname> <port>\n");
		exit(1);
	}
	// open socket and connect to the server
	int sockfd = socky(argv[1],argv[2]);
	int bytes_recv;
	FILE *fp;
	fp = fopen( SAVEDFILE, "w" );
	bytes_recv = receive_data(sockfd, fp);
	fclose(fp);

	printf("[CLIENT] received %d bytes written to %s \n",bytes_recv,SAVEDFILE);
	
	close(sockfd);
	
	return(0);
}
