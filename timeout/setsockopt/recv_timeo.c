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
int socket_fd, port, n, nRcvbuf, nSndbuf, filelength;
struct sockaddr_in srv, cli;
char send_buffer[2+1], recv_buffer[MAXLINE+1], count, file[2];
socklen_t srvlen = sizeof(srv), clilen = sizeof(cli);
FILE* ofile;

int main(int argc, char** argv){
/***			Initialization			***/
	if(argc == 2)
		port = atoi(argv[1]);
	else
		return 1;

	memset(send_buffer, 0, 2+1);
	memset(recv_buffer, 0, MAXLINE+1);
	bzero(&srv, sizeof(srv));
	bzero(&cli, sizeof(cli));

	srv.sin_family = AF_INET;
	srv.sin_port = htons(port);
	srv.sin_addr.s_addr = htonl(INADDR_ANY);
	
	count = 1;
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
	nRcvbuf = 32*1024;
	nSndbuf = 32*1024;
	setsockopt(socket_fd, SOL_SOCKET, SO_RCVBUF, (const char*)&nRcvbuf, sizeof(int));
	setsockopt(socket_fd, SOL_SOCKET, SO_SNDBUF, (const char*)&nSndbuf, sizeof(int));

/***			Work out			***/
	while(n = recvfrom(socket_fd, recv_buffer, MAXLINE, 0, (struct sockaddr*)&cli, &clilen)){
		fprintf(stdout, "[Receiver] Get %d %d %d.\n", filelength, count, recv_buffer[SPP+1]);
		if(recv_buffer[SPP+1] == count){
			if(file[0] == 0){  //  file name
				if((ofile = fopen(recv_buffer, "w")) == NULL){
					fprintf(stdout, "[Receiver] Failed to open the output file %s.\n", recv_buffer);
					return 1;
				}
				fprintf(stdout, "[Receiver] Open %s.\n", recv_buffer);
				file[0] = 1;
			}
			else if(file[1] == 0){  //  file size
				filelength = atoi(recv_buffer);
				fprintf(stdout, "[Receiver] File size %d.\n", filelength);
				file[1] = 1;
			}
			else{
				if((filelength-=SPP) <= 0){
					fwrite(recv_buffer, 1, filelength+SPP, ofile);
					break;
				}
				else
					fwrite(recv_buffer, 1, SPP, ofile);
			}
			send_buffer[1] = count;
			sendto(socket_fd, send_buffer, 2, 0, (struct sockaddr*)&cli, clilen);
			if(count++ == 100)
				count = 1;
		}
		else{
			if(count != 1)
				send_buffer[1] = count-1;
			else
				send_buffer[1] = 100;
			sendto(socket_fd, send_buffer, 2, 0, (struct sockaddr*)&cli, clilen);
		}
		memset(send_buffer, 0, 2);
		memset(recv_buffer, 0, MAXLINE);
	}
	
/***			Close the file			***/
	fprintf(stdout, "[Receiver] Close.\n");
	fclose(ofile);
	return 0;
}
