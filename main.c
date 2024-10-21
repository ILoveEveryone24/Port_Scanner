#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>

enum Arg{
	PROGRAM,
	FLAG,
	IP,
	PORT,
	PORTEND
};

void scan(char *target_ip, int port){
	int sockfd;
	struct sockaddr_in server_addr;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd < 0){
		perror("Error creating socket");
		return;
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
		    return;
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
}

int main(int argc, char *argv[]){
	if(argc < 4 || argc > 5){
		printf("Please provide an IP address and a port as so: %s [FLAG] [IP address] [PORT]\n", argv[0]);
		return 1;
	}

	if(argc == 5){
		if(strcmp(argv[FLAG], "-sR") == 0){
			for(int port = atoi(argv[PORT]); port < atoi(argv[PORTEND])+1; port++){
				scan(argv[IP], port);
			}
		}
	}
	else{
		if(strcmp(argv[FLAG], "-s") == 0){
			scan(argv[IP], atoi(argv[PORT]));
		}
	}

	return 0;
}
