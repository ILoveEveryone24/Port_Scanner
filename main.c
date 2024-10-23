#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <netinet/ip.h>

enum Arg{
	PROGRAM,
	FLAG,
	IP,
	PORT,
	PORTEND
};

struct pseudo_header{
	u_int32_t source_address;
	u_int32_t dest_address;
	u_int8_t placeholder;
	u_int8_t protocol;
	u_int16_t tcp_length;
};

unsigned char checksum(void *b, int len){
	unsigned short *buf = b;
	unsigned int sum = 0;
	unsigned short result;

	for(sum = 0; len > 1; len -= 2){
		sum += *buf++;	
	}
	if(len == 1){
		sum += *(unsigned char *) buf;	
	}

	sum = (sum >> 16) + (sum & 0xFFFF);
	sum += (sum >> 16);
	result = ~sum;
	return result;
}

void stealth_scan(char *target_ip, int port){
	int sock_r;
	struct sockaddr_in server_addr;
	char packet[4096];

	memset(packet, 0, 4096);

	sock_r = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);
	if(sock_r < 0){
		perror("Error creating raw socket");
		return;
	}
	memset(&server_addr, 0, sizeof(server_addr));

	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	server_addr.sin_addr.s_addr = inet_addr(target_ip);

	struct iphdr *iph = (struct iphdr *) packet;

	struct tcphdr *tcph = (struct tcphdr *) (packet + sizeof(struct iphdr));

	iph->ihl = 5;
	iph->version = 4;
	iph->tos = 0;
	iph->tot_len = sizeof(struct iphdr) + sizeof(struct tcphdr);
	iph->id = htonl(12345);
	iph->frag_off = 0;
	iph->ttl = 225;
	iph->protocol = IPPROTO_TCP;
	iph->check = 0;
	iph->saddr = inet_addr("192.168.1.2");
	iph->daddr = server_addr.sin_addr.s_addr;

	tcph->source = htons(12345); 
	tcph->dest = htons(port); 
	tcph->seq = 0;
	tcph->ack_seq = 0;
	tcph->doff = 5; 
	tcph->syn = 1; 
	tcph->window = htons(5840); 
	tcph->check = 0; 
	tcph->urg_ptr = 0;

	struct pseudo_header psh;
	psh.source_address = inet_addr("192.168.1.2");
	psh.dest_address = server_addr.sin_addr.s_addr;
	psh.placeholder = 0;
	psh.protocol = IPPROTO_TCP;
	psh.tcp_length = htons(sizeof(struct tcphdr));

	int psize = sizeof(struct pseudo_header) + sizeof(struct tcphdr);
	char *pseudogram = malloc(psize);
	memcpy(pseudogram, &psh, sizeof(struct pseudo_header));
	memcpy(pseudogram + sizeof(struct pseudo_header), tcph, sizeof(struct tcphdr));

	tcph->check = checksum((unsigned short *) pseudogram, psize);
	iph->check = checksum((unsigned short *) packet, iph->tot_len);

	if(sendto(sock_r, packet, iph->tot_len, 0, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0){
		perror("Error sending packet");
		return;
	}
	else{
		printf("Packet sent!\n");	
	}

	close(sock_r);
	free(pseudogram);
}

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
