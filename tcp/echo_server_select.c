#include "server.h"
int main(int argc, char** argv){
	/********************DEFINITION********************/
	int socket_fd, accept_fd, maxfdp1, maxi = -1, i, j, nready, sockfd;
	struct sockaddr_in srv;
	char receive_buffer[800], send_buffer[800], client_fd[MAXCLIENT], IP_buff[16];  //  client_fd is used to collect client fd
	bzero(&IP_buff, sizeof(IP_buff));
	memset(client_fd, -1, sizeof(client_fd));   //  initialize client_fd
	memset(receive_buffer, 0, sizeof(receive_buffer));
	memset(send_buffer, 0, sizeof(send_buffer));
	struct sockaddr_in cli;
	socklen_t addrlen = sizeof(cli);

	/********************DEFINITION********************/

	/********************PREPARING********************/
	if(server_setup(&socket_fd, &srv) == -1)
		return -1;

	maxfdp1 = socket_fd + 1;  //  this record the max fd in the fdset
	fd_set ready_fdset, client_fdset;
	FD_ZERO(&ready_fdset);
	FD_ZERO(&client_fdset);  //  reset fdset
	FD_SET(socket_fd, &client_fdset);

	/********************PREPARING********************/
	
	/********************FUNCTION********************/
	while(1){
		ready_fdset = client_fdset;  //  why??  --  same as client
		nready = select(maxfdp1, &ready_fdset, NULL, NULL, NULL);  //  stock here
		if(FD_ISSET(socket_fd, &ready_fdset)){
			/********************ACCEPTION********************/
			accept_fd = accept(socket_fd, (struct sockaddr*)&cli, &addrlen);
			if(accept_fd == -1){
				fprintf(stderr, "[Server] Error! Accept failed! accept_fd == %d\n", accept_fd);
				return -1;
			}
			else{
				inet_ntop(AF_INET, &cli.sin_addr.s_addr, IP_buff, sizeof(IP_buff));
				fprintf(stderr, "[Server] Success accept a client from %s port %d!\n", IP_buff, ntohs(cli.sin_port));
			}

			/********************ACCEPTION********************/
		
			/********************INITIALIZE A CLIENT********************/
			for(i = 0; i < MAXCLIENT; i++)
				if(client_fd[i] == -1){
					client_fd[i] = accept_fd;
					break;
				}
			if(i == MAXCLIENT){
				fprintf(stderr, "[Server] Client maximum number %d is reached, can't accept anymore.", MAXCLIENT);
				close(accept_fd);
				continue;
			}
			FD_SET(accept_fd, &client_fdset);
			++maxi;
			if(maxfdp1 < accept_fd + 1)
				maxfdp1 = accept_fd + 1;
			if(--nready == 0)
				continue;

			/********************INITIALIZE A CLIENT********************/
		}
		for(i = 0; i <= maxi; i++){
			if(client_fd[i] < 0)
				continue;
			if(FD_ISSET(client_fd[i], &ready_fdset)){  //  if socket is readable
				if(read(client_fd[i], receive_buffer, sizeof(receive_buffer)) == 0 || strcmp(receive_buffer, "exit\n") == 0 || strcmp(receive_buffer, "exit\r\n") == 0 || strlen(receive_buffer) == 0){
					close(client_fd[i]);
					FD_CLR(client_fd[i], &client_fdset);
					client_fd[i] = -1;
					while(client_fd[maxi] == -1){
						--maxi;
						if(maxi == -1)
							break;
					}
					fprintf(stderr, "[Server] Connection to %s closed!\n", IP_buff);
				}
				else{
					strcpy(send_buffer, receive_buffer);
					write(client_fd[i], send_buffer, sizeof(send_buffer));
				}
				if(--nready == 0)
					break;
			}
		}
	}
	/********************FUNCTION********************/

	/********************END********************/
	close(socket_fd);
	return 0;

	/********************END********************/
}
