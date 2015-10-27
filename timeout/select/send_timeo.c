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

int readable_timeo(int socket_fd, int sec, int usec);

/***			Declaration			***/
int socket_fd, port, n, nSndbuf, nRcvbuf, filelength, usec;
char *ip, send_buffer[MAXLINE+1], recv_buffer[2+1], *ifilename, *ofilename, count, *temp, file[2];
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
/*		if((SPP = atoi(argv[6])) > 1000){
			fprintf(stdout, "./Send_timeo [IP] [port] [ifilename] [ofilename] [usec/resend] [SPP <= 1000]\n");
			return 1;
		}*/
	}
	else{
		fprintf(stdout, "./Send_timeo [IP] [port] [ifilename] [ofilename] [usec/resend] [SPP <= 1000]\n");
		return 1;
	}

	bzero(&srv, sizeof(srv));

	memset(send_buffer, 0, MAXLINE+1);
	memset(recv_buffer, 0, 2+1);
	srv.sin_family = AF_INET;
	srv.sin_port = htons(port);
	inet_pton(AF_INET, ip, &srv.sin_addr);

	count = 1;
	file[0] = 0;
	file[1] = 0;

/***			Set environment			***/
	if((ifile = fopen(ifilename, "r")) == NULL){
		fprintf(stdout, "[Sender] Failed to open the input file.\n");
		return 1;
	}
	if(fseek(ifile, 0, SEEK_END)){
		fprintf(stdout, "[Sender] Failed to seek th input file.\n");
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
	nRcvbuf = 32*1024;
	nSndbuf = 32*1024;
	setsockopt(socket_fd, SOL_SOCKET, SO_RCVBUF, (const char*)&nRcvbuf, sizeof(int));
	setsockopt(socket_fd, SOL_SOCKET, SO_SNDBUF, (const char*)&nSndbuf, sizeof(int));

/***			Work out			***/
	filelength = ftell(ifile);
	rewind(ifile);
	
	while(file[0] == 0){
		sprintf(send_buffer, "%s", ofilename);
		fprintf(stdout, "[Sender] Continue %s.\n", ofilename);
		send_buffer[SPP+1] = count;
		write(socket_fd, send_buffer, MAXLINE);
		file[0] = 1;
		while(1){
			if(readable_timeo(socket_fd, 0, usec) <= 0){
				write(socket_fd, send_buffer, MAXLINE);
				fprintf(stdout, "[Sender] Resend %d %d.\n", count, send_buffer[SPP+1]);
			}
			else if(read(socket_fd, recv_buffer, 2) > 0){
				if(recv_buffer[1] == count){
					alarm(0);
					if(count++ == 100)
						count = 1;
					break;
				}
			}
			else{
				if(filelength <= 0)
					break;
			}
		}
		memset(send_buffer, 0, MAXLINE);
		memset(recv_buffer, 0, 2);
	}
	while(file[1] == 0){
		sprintf(send_buffer, "%d", filelength);
		fprintf(stdout, "[Sender] Continue %d.\n", filelength);
		send_buffer[SPP+1] = count;
		write(socket_fd, send_buffer, MAXLINE);
		file[1] = 1;
		while(1){
			if(readable_timeo(socket_fd, 0, usec) <= 0){
				write(socket_fd, send_buffer, MAXLINE);
				fprintf(stdout, "[Sender] Resend %d %d.\n", count, send_buffer[SPP+1]);
			}
			else if(read(socket_fd, recv_buffer, 2) > 0){
				if(recv_buffer[1] == count){
					alarm(0);
					if(count++ == 100)
						count = 1;
					break;
				}
			}
			else{
				if(filelength <= 0)
					break;
			}
		}
		memset(send_buffer, 0, MAXLINE);
		memset(recv_buffer, 0, 2);
	}
	while((n = fread(send_buffer, 1, SPP, ifile)) != 0){
		send_buffer[SPP+1] = count;
		write(socket_fd, send_buffer, MAXLINE);
		fprintf(stdout, "[Sender] Send %d %d.\n", count, send_buffer[SPP+1]);
		filelength-=SPP;
		while(1){
			if(readable_timeo(socket_fd, 0, usec) <= 0){
				write(socket_fd, send_buffer, MAXLINE);
				fprintf(stdout, "[Sender] Resend %d %d.\n", count, send_buffer[SPP+1]);
			}
			else if(read(socket_fd, recv_buffer, 2) > 0){
				fprintf(stdout, "[Sender] Back %d %d.\n", count, recv_buffer[1]);
				if(filelength <= 0)
					break;
				if(recv_buffer[1] == count){
					alarm(0);
					if(count++ == 100)
						count = 1;
					break;
				}
			}
			else{
				if(filelength <= 0)
					break;
			}
		}
		memset(send_buffer, 0, MAXLINE);
		memset(recv_buffer, 0, 2);
	}
	
/***			Close the file			***/
	fprintf(stdout, "[Sender] Close.\n");
	fclose(ifile);
	return 0;
}

int readable_timeo(int socket_fd, int sec, int usec)
{
	fd_set ready_fdset;
	struct timeval tv;
	
	FD_ZERO(&ready_fdset);
	FD_SET(socket_fd, &ready_fdset);
	
	tv.tv_sec = sec;
	tv.tv_usec = usec;

	return (select(socket_fd+1, &ready_fdset, NULL, NULL, &tv));
}
