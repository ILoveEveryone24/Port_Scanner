#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>


int main(){
	int sockfd;
	struct sockaddr_in server_addr;

	char *target_ip = "127.0.0.1";
	int port = 0;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd < 0){
		perror("Error creating socket");
		return -1;	
	}
	for(int i = 0; i < 640; i++){
		port = i;
		memset(&server_addr, 0, sizeof(server_addr));
		server_addr.sin_family = AF_INET;
		server_addr.sin_port = htons(port);

		if(inet_pton(AF_INET, target_ip, &server_addr.sin_addr) < 1){
			perror("Invalid IP address");
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

	}
	close(sockfd);

	return 0;
}
