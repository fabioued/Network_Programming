#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <sys/types.h>

#include <signal.h>
#include <errno.h>

#define MAXLINE 1002
#define SPP 1000

/***			Declaration			***/
int socket_fd, port, n, nRcvbuf, nSndbuf, filelength, i;
struct sockaddr_in srv, cli;
char check[2][50], send_buffer[2+1], recv_buffer[50][MAXLINE+1], count[2], file[2];
socklen_t srvlen = sizeof(srv), clilen = sizeof(cli);
FILE* ofile;

int main(int argc, char** argv){
/***			Initialization			***/
	if(argc == 2)
		port = atoi(argv[1]);
	else
		return 1;
	
	memset(check, 0, 50*2);
	memset(send_buffer, 0, 2+1);
	memset(recv_buffer, 0, 50*(MAXLINE+1));
	bzero(&srv, sizeof(srv));
	bzero(&cli, sizeof(cli));

	srv.sin_family = AF_INET;
	srv.sin_port = htons(port);
	srv.sin_addr.s_addr = htonl(INADDR_ANY);
	
	i = 0;
	count[0] = 1;
	count[1] = 1;
	file[0] = 0;
	file[1] = 0;

/***			Set environment			***/
	if((socket_fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1){
		fprintf(stdout, "[Receiver] Failed to create socket.\n");
		return 1;
	}
	if(bind(socket_fd, (struct sockaddr*)&srv, srvlen) == -1){
		fprintf(stdout, "[Receiver] Failed to bind socket.\n");
		return 1;
	}
	fprintf(stdout, "[Receiver] Success.\n");

/***			Socket options			***/
	nSndbuf = 32*1024;
	nRcvbuf = 32*1024;
	setsockopt(socket_fd, SOL_SOCKET, SO_SNDBUF, (const char*)&nSndbuf, sizeof(int));
	setsockopt(socket_fd, SOL_SOCKET, SO_RCVBUF, (const char*)&nRcvbuf, sizeof(int));

/***			Work out			***/
	while(n = recvfrom(socket_fd, recv_buffer[count[0]-1], MAXLINE, 0, (struct sockaddr*)&cli, &clilen)){
		if((recv_buffer[count[0]-1][SPP+1] == count[0]) && (recv_buffer[count[0]-1][SPP+2] == count[1])){
			if(file[0] == 0){  //  file name
				if((ofile = fopen(recv_buffer[count[0]-1], "w")) == NULL){
					fprintf(stdout, "[Receiver] Failed to open the output file %s.\n", recv_buffer[count[0]-1]);
					return 1;
				}
				fprintf(stdout, "[Receiver] Open %s.\n", recv_buffer[count[0]-1]);
				file[0] = 1;
			}
			else if(file[1] == 0){  //  file size
				filelength = atoi(recv_buffer[count[0]-1]);
				fprintf(stdout, "[Receiver] File size %d.\n", filelength);
				file[1] = 1;
				break;
			}
			send_buffer[1] = count[0];
			send_buffer[2] = count[2];
			sendto(socket_fd, send_buffer, 3, 0, (struct sockaddr*)&cli, clilen);
			if(count[0]++ == 50)
				count[0] = 1;
		}
		else
			return 1;
		memset(send_buffer, 0, 3);
		memset(recv_buffer, 0, 50*(MAXLINE+1));
	}
	while(n = recvfrom(socket_fd, recv_buffer[count[0]-1], MAXLINE, 0, (struct sockaddr*)&cli, &clilen)){
		if((check[0][recv_buffer[count[0]-1][SPP+1]] == 0) && (recv_buffer[count[0]-1][SPP+2] == count[1]))
	
/***			Close the file			***/
	fprintf(stdout, "[Receiver] Close.\n");
	fclose(ofile);
	return 0;
}
