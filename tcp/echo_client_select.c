#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>
#include <netinet/in.h>  //  sockaddr_in
#include <sys/select.h>  //  select
#include <unistd.h>

#define PORT 6556
#define BACKLOG 2
int main(int argc, char** argv){
	int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if(socket_fd == -1){
		fprintf(stderr, "Client : Error! Can't create socket! socket == %d\n", socket_fd);
		return -1;
	}
	else
		fprintf(stderr, "Client : Success create socket!\n");
	
	struct sockaddr_in srv;
	srv.sin_family = AF_INET;
	srv.sin_port = htons(PORT);
	srv.sin_addr.s_addr = inet_addr("140.113.235.147");
	
	//  no bind means no specific port number
	char IP_buff[16];
	inet_ntop(AF_INET, &srv.sin_addr.s_addr, IP_buff, sizeof(IP_buff));
	int connect_fd = connect(socket_fd,(struct sockaddr*)&srv, sizeof(srv));
	if(connect_fd == -1){
		fprintf(stderr, "Client : Error! Can't connect to server! connect_fd == %d\n", connect_fd);
		return -1;
	}
	else
		fprintf(stderr, "Client : Success connect to server %s!\n", IP_buff);
	
	char receive_buffer[800], send_buffer[800];
	memset(receive_buffer, 0, sizeof(receive_buffer));
	memset(send_buffer, 0, sizeof(send_buffer));
	fd_set client_fdset;  //  read and write
	FD_ZERO(&client_fdset);
	fprintf(stderr, "Client : Enter some words : ");
	while(1){
		FD_SET(fileno(stdin), &client_fdset);  //  why must in while loop?
		FD_SET(socket_fd, &client_fdset);  //  also  --  same as server ready_fdset = client_fdset
		select(socket_fd + 1, &client_fdset, NULL, NULL, NULL);
		if(FD_ISSET(fileno(stdin), &client_fdset)){
			fgets(send_buffer, sizeof(send_buffer), stdin);
			write(socket_fd, send_buffer, sizeof(send_buffer));
//			if(strcmp(send_buffer, "exit\n") == 0){
//				fprintf(stderr, "Client : Connection close!\n");
//				break;
//			}
		}
		if(FD_ISSET(socket_fd, &client_fdset)){
			if((read(socket_fd, receive_buffer, sizeof(receive_buffer))) == 0){
				fprintf(stderr, "Client : Connection closed by server %s!\n", IP_buff);
				FD_CLR(socket_fd, &client_fdset);
				break;
			}
			fprintf(stderr, "Client : Received from server : %s", receive_buffer);
			fprintf(stderr, "Client : Enter some words : ");
		}
	}

	close(socket_fd);
	return 0;
}
