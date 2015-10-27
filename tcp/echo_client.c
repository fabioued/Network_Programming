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
	int connect_fd = connect(socket_fd,(struct sockaddr*)&srv, sizeof(srv));
	if(connect_fd == -1){
		fprintf(stderr, "Client : Error! Can't connect to server! connect_fd == %d\n", connect_fd);
		return -1;
	}
	else
		fprintf(stderr, "Client : Success connect to server!\n");

	while(1){
		char receive_buffer[800], send_buffer[800];
		memset(receive_buffer, 0, sizeof(receive_buffer));
		memset(send_buffer, 0, sizeof(send_buffer));
		fprintf(stderr, "Client : Enter some words : ");
		fgets(send_buffer, sizeof(send_buffer), stdin);
		write(socket_fd, send_buffer, sizeof(send_buffer));
		if(strcmp(send_buffer, "exit\n") == 0)break;
		read(socket_fd, receive_buffer, sizeof(receive_buffer));
		fprintf(stderr, "Client : Received from server : %s", receive_buffer);
	}

	close(socket_fd);
	fprintf(stderr, "Client : Connection close!\n");
	return 0;
}
