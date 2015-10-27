#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>
#include <netinet/in.h>  //  sockaddr_in
#include <sys/select.h>  //  select
#include <unistd.h>
int client_setup(int* socket_fd, struct sockaddr_in* srv, char* srv_IP, int port)
{
	*socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if(*socket_fd == -1){
		fprintf(stderr, "[Client] ERROR : Can't create socket! socket == %d\n", *socket_fd);
		return -1;
	}
	else
		fprintf(stderr, "[Client] SUCCESS : Successfully create socket!\n");
	
	bzero(srv, sizeof(*srv));
	srv->sin_family = AF_INET;
	srv->sin_port = htons(port);
	srv->sin_addr.s_addr = inet_addr(srv_IP);
}
