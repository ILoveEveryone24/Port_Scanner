#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>


int main(int argc, char *argv[]){
	int sockfd;
	struct sockaddr_in server_addr;

	if(argc != 3){
		printf("Please provide an IP address and a port as so: %s [IP address] [PORT]\n", argv[0]);
		return 1;
	}

	char *target_ip = argv[1];
	int port = atoi(argv[2]);

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd < 0){
		perror("Error creating socket");
		return -1;	
	}
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);

	if(inet_pton(AF_INET, target_ip, &server_addr.sin_addr) < 1){
		 if (errno == EINVAL) {
			perror("Invalid IP address");
		    } else {
			printf("Invalid IP address format.\n");
		    }
		    return -1;
	}

	if(connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0){
		if(errno == ECONNREFUSED){
			printf("Port %d is closed.\n", port);	
		}
		else if(errno == ETIMEDOUT){
			printf("Port %d is filtered.\n", port);	
		}
		else{
			printf("Port %d: Error %s\n", port, strerror(errno));	
		}
	}
	else{
		printf("Port %d is open.\n", port);	
	}

	close(sockfd);

	return 0;
}
