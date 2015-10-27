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
	struct sockaddr_in srv, cli;
	char send_buffer[MAXLINE+1], recv_buffer[MAXLINE+1], *ofilename;
	socklen_t srvlen = sizeof(srv), clilen = sizeof(cli);
	FILE* ofile;

/***			Initialization			***/
	if(argc == 3){
		port = atoi(argv[1]);
		ofilename = argv[2];
	}

	memset(send_buffer, 0, MAXLINE+1);
	memset(recv_buffer, 0, MAXLINE+1);
	bzero(&srv, sizeof(srv));
	bzero(&cli, sizeof(cli));

	srv.sin_family = AF_INET;
	srv.sin_port = htons(port);
	srv.sin_addr.s_addr = htonl(INADDR_ANY);

/***			Set environment			***/
	if((ofile = fopen(ofilename, "w")) == NULL){
		fprintf(stdout, "[Server] Failed to open the output file.\n");
		return 1;
	}
	if((socket_fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1){
		fprintf(stdout, "[Server] Failed to create socket.\n");
		return 1;
	}
	if(bind(socket_fd, (struct sockaddr*)&srv, srvlen) == -1){
		fprintf(stdout, "[Server] Failed to bind socket.\n");
		return 1;
	}
	fprintf(stdout, "[Server] Success.\n");

/***			Work out			***/
	while(recvfrom(socket_fd, recv_buffer, MAXLINE, 0, (struct sockaddr*)&cli, &clilen)){
//		sendto(socket_fd, send_buffer, strlen(send_buffer), 0, (struct sockaddr*)&cli, clilen);
		fprintf(ofile, "%s", recv_buffer);
		memset(recv_buffer, 0, strlen(recv_buffer));
	}
	
/***			Close the file			***/
	fprintf(stdout, "[Server] Close.\n");
	fclose(ofile);
	return 0;
}
