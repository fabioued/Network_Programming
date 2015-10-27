#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>
#include <netinet/in.h>  //  sockaddr_in
#include <unistd.h>

#define PORT 6556
#define BACKLOG 2
int main(int argc, char** argv){
	int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if(socket_fd == -1){
		fprintf(stderr, "Server : Error! Can't create socket! socket == %d\n", socket_fd);
		return -1;
	}
	else
		fprintf(stderr, "Server : Success create socket!\n");
	
	struct sockaddr_in srv;
	bzero(&srv, sizeof(srv));
	srv.sin_family = AF_INET;
	srv.sin_port = htons(PORT);
	srv.sin_addr.s_addr = inet_addr("140.113.235.147");
	
	int bind_fd = bind(socket_fd,(struct sockaddr*)&srv, sizeof(srv));
	if(bind_fd == -1){
		fprintf(stderr, "Server : Error! Can't bind socket! bind_fd == %d\n", bind_fd);
		return -1;
	}
	else
		fprintf(stderr, "Server : Success bind socket!\n");
	
	int listen_fd = listen(socket_fd, BACKLOG);  //  BACKLOG = 2 which means can concurrently receive 2 connection request. If the connection request is more than 2, then client will get ECONNREFUSED error.
	if(listen_fd == -1){
		fprintf(stderr, "Server : Error! Can't start to listen! listen_fd == %d\n", listen_fd);
		return -1;
	}
	else
		fprintf(stderr, "Server : Success start to listen!\n");

	char IP_buff[16];
	bzero(&IP_buff, sizeof(IP_buff));
	struct sockaddr_in cli;
	socklen_t addrlen = sizeof(cli);
	while(1){
		int accept_fd = accept(socket_fd, (struct sockaddr*)&cli, &addrlen);
		if(accept_fd == -1){
			fprintf(stderr, "Server : Error! Accept failed! accept_fd == %d\n", accept_fd);
			return -1;
		}
		else{
			inet_ntop(AF_INET, &cli.sin_addr.s_addr, IP_buff, sizeof(IP_buff));
			fprintf(stderr, "Server : Success accept a client from %s port %d!\n", IP_buff, ntohs(cli.sin_port));  //  show IP address and PORT number
		}
	
		char receive_buffer[800], send_buffer[800];
		while(1){
			memset(receive_buffer, 0, sizeof(receive_buffer));
			memset(send_buffer, 0, sizeof(send_buffer));
			read(accept_fd, receive_buffer, sizeof(receive_buffer));
			if(strcmp(receive_buffer, "exit\n") == 0)break;
			strcpy(send_buffer, receive_buffer);
			strcat(send_buffer,"\r\n");
			write(accept_fd, receive_buffer, sizeof(send_buffer));
		}
		fprintf(stderr, "Server : Connection closed by client!\n");
	}
	close(socket_fd);
	return 0;
}
