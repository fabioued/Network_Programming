#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <sys/types.h>

#define MAXLINE 100

int main(int argc, char** argv){
/***			Declaration			***/
	int socket_fd, port;
	char *ip, send_buffer[MAXLINE+1], recv_buffer[MAXLINE+1], *ifilename;
	struct sockaddr_in srv;
	socklen_t srvlen = sizeof(srv);
	FILE* ifile;

/***			Initialization			***/
	if(argc == 4){
		ip = argv[1];
		port = atoi(argv[2]);
		ifilename = argv[3];
	}

	bzero(&srv, sizeof(srv));

	memset(send_buffer, 0, MAXLINE+1);
	memset(recv_buffer, 0, MAXLINE+1);
	srv.sin_family = AF_INET;
	srv.sin_port = htons(port);
	inet_pton(AF_INET, ip, &srv.sin_addr);

/***			Set environment			***/
	if((ifile = fopen(ifilename, "r")) == NULL){
		fprintf(stdout, "[Client] Failed to open the input file.\n");
		return 1;
	}
	if((socket_fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1){
		fprintf(stdout, "[Client] Failed to create socket.\n");
		return 1;
	}
	if(connect(socket_fd, (struct sockaddr*)&srv, srvlen) == -1){
		fprintf(stdout, "[Client] Failed to connect to server.\n");
		return 1;
	}
	fprintf(stdout, "[Client] Success.\n");

/***			Work out			***/
	while(fgets(send_buffer, MAXLINE, ifile)){
//		sendto(socket_fd, send_buffer, strlen(send_buffer), 0, (struct sockaddr*)&srv, srvlen);
//		recvfrom(socket_fd, recv_buffer, MAXLINE, 0, (struct sockaddr*)&srv, srvlen);
		write(socket_fd, send_buffer, strlen(send_buffer));  /* when using connect() */
		memset(send_buffer, 0, strlen(send_buffer));
	}
	
/***			Close the file			***/
	fprintf(stdout, "[Client] Close.\n");
	fclose(ifile);
	return 0;
}
