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

static void sig_alrm(int signo);

/***			Declaration			***/
int socket_fd, port, n, nSndbuf, nRcvbuf, filelength, usec, i, j, finish;
char *ip, check[2][50], send_buffer[50][MAXLINE+1], recv_buffer[2+1], *ifilename, *ofilename, count[2], *temp, file[2];
struct sockaddr_in srv;
socklen_t srvlen = sizeof(srv);
FILE* ifile;

int main(int argc, char** argv){
/***			Initialization			***/
	if(argc == 6){
		ip = argv[1];
		port = atoi(argv[2]);
		ifilename = argv[3];
		ofilename = argv[4];
		if((usec = atoi(argv[5])) >= 1000000){
			fprintf(stdout, "./Send_timeo [IP] [port] [ifilename] [ofilename] [usec/resend] [SPP <= 1000]\n");
			return 1;
		}
/*		if(SPP = atoi(argv[6]) > 1000){
			fprintf(stdout, "./Send_timeo [IP] [port] [ifilename] [ofilename] [usec/resend] [SPP <= 1000]\n");
			return 1;
		}*/
	}
	else{
		fprintf(stdout, "./Send_timeo [IP] [port] [ifilename] [ofilename] [usec/resend] [SPP <= 1000]\n");
		return 1;
	}

	bzero(&srv, sizeof(srv));
	
	memset(check, 0, 2*50);
	memset(send_buffer, 0, 50*(MAXLINE+1));
	memset(recv_buffer, 0, 2+1);
	srv.sin_family = AF_INET;
	srv.sin_port = htons(port);
	inet_pton(AF_INET, ip, &srv.sin_addr);
	
	finish = 0;
	count[0] = 1;
	count[1] = 1;
	file[0] = 0;
	file[1] = 0;

/***			Set environment			***/
	if((ifile = fopen(ifilename, "r")) == NULL){
		fprintf(stdout, "[Sender] Failed to open the input file.\n");
		return 1;
	}
	if(fseek(ifile, 0, SEEK_END)){
		fprintf(stdout, "[Sender] Failed to seek the input file.\n");
		return 1;
	}
	if((socket_fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1){
		fprintf(stdout, "[Sender] Failed to create socket.\n");
		return 1;
	}
	if(connect(socket_fd, (struct sockaddr*)&srv, srvlen) == -1){
		fprintf(stdout, "[Sender] Failed to connect to server.\n");
		return 1;
	}
	fprintf(stdout, "[Sender] Success.\n");

/***			Socket options			***/
	nSndbuf = 32*1024;
	nRcvbuf = 32*1024;
	setsockopt(socket_fd, SOL_SOCKET, SO_SNDBUF, (const char*)&nSndbuf, sizeof(int));
	setsockopt(socket_fd, SOL_SOCKET, SO_RCVBUF, (const char*)&nRcvbuf, sizeof(int));

/***			Work out			***/
	filelength = ftell(ifile);
	rewind(ifile);
	signal(SIGALRM, sig_alrm);
	
	while(file[0] == 0){
		sprintf(send_buffer[0], "%s", ofilename);
		fprintf(stdout, "[Sender] Continue %s.\n", ofilename);
		send_buffer[0][SPP+1] = count[0];
		write(socket_fd, send_buffer[0], MAXLINE);
		file[0] = 1;
		ualarm(usec, 0);
		while(read(socket_fd, recv_buffer, 2) > 0){
			if(recv_buffer[1] == count[0]){
				ualarm(0, 0);
				if(count[0]++ == 100)
					count = 1;
				break;
			}
		}
		memset(send_buffer[0], 0, MAXLINE);
		memset(recv_buffer, 0, 2);
	}
	while(file[1] == 0){
		sprintf(send_buffer[0], "%d", filelength);
		fprintf(stdout, "[Sender] Continue %d.\n", filelength);
		send_buffer[0][SPP+1] = count[0];
		write(socket_fd, send_buffer[0], MAXLINE);
		file[1] = 1;
		ualarm(usec, 0);
		while(read(socket_fd, recv_buffer, 2) > 0){
			if(recv_buffer[1] == count){
				ualarm(0, 0);
				if(count[0]++ == 100)
					count[0] = 1;
				break;
			}
		}
		memset(send_buffer[0], 0, MAXLINE);
		memset(recv_buffer, 0, 2);
	}
	while(!finish){
		for(i = count[0]-1; i < 50; i++){
			if(fread(send_buffer[i], 1, SPP, ifile) == 0){
				finish = 1;
				break;
			}
			else{
				send_buffer[i][SPP+1] = count[0]++;
				send_buffer[i][SPP+2] = count[1];
				write(socket_fd, send_buffer[i], MAXLINE);
			}
		}
		ualarm(usec, 0);

		while(read(socket_fd, recv_buffer, 3) > 0){
			if(filelength <= 0)
				break;
			if((check[0][recv_buffer[1]-1] == 1) && (check[1][recv_buffer[1]-1] == 0) && (recv_buffer[2] == count[1])){
				check[1][recv_buffer[1]-1] = 1;
				filelength -= SPP;
				if(--i == 0){
					ualarm(0, 0);
					count[0] = 1;
					if(count[1]++ == 100)
						count[1] = 1;
					break;
				}
			}
		}
		memset(check, 0, 2*50);
		memset(send_buffer, 0, 50*(MAXLINE+1));
		memset(recv_buffer, 0, 2+1);
	}
/*	while((n = fread(send_buffer, 1, SPP, ifile)) != 0){
		send_buffer[SPP+1] = count;
		write(socket_fd, send_buffer, MAXLINE);
		ualarm(usec, 0);

		while(read(socket_fd, recv_buffer, 2) > 0){
			if(filelength <= 0)
				break;
			if(recv_buffer[1] == count){
				filelength-=SPP;
				ualarm(0, 0);
				if(count++ == 100)
					count = 1;
				break;
			}
		}
		memset(send_buffer, 0, MAXLINE);
		memset(recv_buffer, 0, 2);
	}*/
	
/***			Close the file			***/
	fprintf(stdout, "[Sender] Close.\n");
	fclose(ifile);
	return 0;
}

static void sig_alrm(int signo)
{
	fprintf(stdout, "[Sender] Resend %d %d.\n", count, send_buffer[SPP+1]);
	for(j = 0; j < 50; i++){
		if((check[0][j] == 1) && (check[1][j] == 0))
			write(socket_fd, send_buffer[j], MAXLINE);
	}
	ualarm(usec, 0);
	return;
}
