#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>
#include <netinet/in.h>  //  sockaddr_in
#include <sys/select.h>  //  select
#include <unistd.h>

#define BACKLOG 20
#define MAXCLIENT 20
int server_setup(int* socket_fd, struct sockaddr_in* srv, int port){
	*socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if(*socket_fd == -1){
		fprintf(stderr, "[Server] Error! Can't create socket! socket == %d\n", *socket_fd);
		return -1;
	}
	else
		fprintf(stderr, "[Server] Success create socket!\n");
	
	bzero(srv, sizeof(*srv));
	srv->sin_family = AF_INET;
	srv->sin_port = htons(port);
	srv->sin_addr.s_addr = inet_addr("0.0.0.0");
	
	int bind_fd = bind(*socket_fd,(struct sockaddr*)srv, sizeof(*srv));
	if(bind_fd == -1){
		fprintf(stderr, "[Server] Error! Can't bind socket! bind_fd == %d\n", bind_fd);
		return -1;
	}
	else
		fprintf(stderr, "[Server] Success bind socket!\n");
	
	int listen_fd = listen(*socket_fd, BACKLOG);
	if(listen_fd == -1){
		fprintf(stderr, "[Server] Error! Can't start to listen on %d! listen_fd == %d\n", port, listen_fd);
		return -1;
	}
	else
		fprintf(stderr, "[Server] Success start to listen on %d!\n", port);
}
