#include "client.h"
int main(int argc, char** argv){
	int socket_fd, connect_fd;
	char* temp;
	char IP_buff[16], receive_buffer[800], send_buffer[800];
	memset(receive_buffer, 0, sizeof(receive_buffer));
	memset(send_buffer, 0, sizeof(send_buffer));
	struct sockaddr_in srv;
	
	if(argc != 3){
		fprintf(stderr, "[Client] Usage: chatroom_client <server_ip> <server_port>\n");
		return 0;
	}

	int port = atoi(argv[2]);
	if(client_setup(&socket_fd, &srv, argv[1], port) == -1)
		return -1;
	
	inet_ntop(AF_INET, &srv.sin_addr.s_addr, IP_buff, sizeof(IP_buff));
	connect_fd = connect(socket_fd, (struct sockaddr*)&srv, sizeof(srv));
	if(connect_fd == -1){
		fprintf(stderr, "[Client] ERROR : Can't connect to server! connect_fd == %d\n", connect_fd);
		return -1;
	}
	else
		fprintf(stderr, "[Client] SUCCESS : Successfully connect to server %s!\n", IP_buff);
	
	fd_set client_fdset;
	FD_ZERO(&client_fdset);
	while(1){
		FD_SET(fileno(stdin), &client_fdset);
		FD_SET(socket_fd, &client_fdset);
		select(socket_fd + 1, &client_fdset, NULL, NULL, NULL);
		if(FD_ISSET(fileno(stdin), &client_fdset)){
			fgets(send_buffer, sizeof(send_buffer), stdin);
			write(socket_fd, send_buffer, sizeof(send_buffer));

			temp = strtok(send_buffer, " ");
			if(strcmp(temp, "exit\n") == 0 || strcmp(temp, "exit") == 0){  /* locally terminate */
				fprintf(stderr, "[Client] Connection close!\n");
				FD_CLR(socket_fd, &client_fdset);
				break;
			}
		}
		if(FD_ISSET(socket_fd, &client_fdset)){
			if((read(socket_fd, receive_buffer, sizeof(receive_buffer))) == 0){  /* remotely terminate */
//				fprintf(stderr, "[Client] Connection closed by server %s!\n", IP_buff);
//				FD_CLR(socket_fd, &client_fdset);
//				break;
			}
			fprintf(stderr, "%s", receive_buffer);
		}
	}
	
	close(socket_fd);
	return 0;
}
