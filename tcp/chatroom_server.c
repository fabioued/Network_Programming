#include "server.h"
int main(int argc, char** argv){
	/********************DEFINITION********************/
	int socket_fd, accept_fd, maxfdp1, maxi = -1, i, j, nready, sockfd, client_fd[MAXCLIENT];
	struct sockaddr_in srv;
	char receive_buffer[800], send_buffer[800], You[800], token_buffer[800], IP_buff[MAXCLIENT][16], client_nick[MAXCLIENT][20], old_nick[20], PORT_buff[MAXCLIENT][8];  //  client_fd is used to collect client fd
	char* temp;
	memset(client_fd, -1, sizeof(client_fd));
	memset(client_nick, 0, sizeof(client_nick));
	struct sockaddr_in cli;
	socklen_t addrlen = sizeof(cli);

	/********************DEFINITION********************/

	/********************PREPARING********************/
	if(argc != 2){
		fprintf(stderr, "[Server] Usage: chatroom_server <server_port>\n");
		return 0;
	}

	int port = atoi(argv[1]);
	if(server_setup(&socket_fd, &srv, port) == -1)
		return -1;

	maxfdp1 = socket_fd + 1;  //  this record the max fd in the fdset
	fd_set ready_fdset, client_fdset;
	FD_ZERO(&ready_fdset);
	FD_ZERO(&client_fdset);  //  reset fdset
	FD_SET(socket_fd, &client_fdset);

	/********************PREPARING********************/
	
	/********************FUNCTION********************/
	while(1){
		ready_fdset = client_fdset;
		nready = select(maxfdp1, &ready_fdset, NULL, NULL, NULL);
		if(FD_ISSET(socket_fd, &ready_fdset)){

			/********************INITIALIZE A CLIENT********************/
			memset(receive_buffer, 0, sizeof(receive_buffer));
			memset(send_buffer, 0, sizeof(send_buffer));
			memset(You, 0, sizeof(You));
			accept_fd = accept(socket_fd, (struct sockaddr*)&cli, &addrlen);
			if(accept_fd == -1){
				fprintf(stderr, "[Server] Error! Accept failed! accept_fd == %d\n", accept_fd);
				return -1;
			}
			else{
				for(i = 0; i < MAXCLIENT; i++){
					if(client_fd[i] == -1){
						client_fd[i] = accept_fd;
						sprintf(client_nick[i], "anonymous");
						break;
					}
				}
				inet_ntop(AF_INET, &cli.sin_addr.s_addr, IP_buff[i], sizeof(IP_buff[i]));
				sprintf(PORT_buff[i], "%d", ntohs(cli.sin_port));
				if(i == MAXCLIENT){
					fprintf(stderr, "[Server] Client maximum number %d is reached, can't accept anymore.\n", MAXCLIENT);
					close(accept_fd);
					continue;
				}
				FD_SET(accept_fd, &client_fdset);
				++maxi;
				if(maxfdp1 < accept_fd + 1)
					maxfdp1 = accept_fd + 1;

				fprintf(stderr, "[Server] Successfully accept a client from %s port %s!\n", IP_buff[i], PORT_buff[i]);
				sprintf(You, "[Server] Hello %s, welcome to 0116042's chatroom server. From %s/%s\n", client_nick[i], IP_buff[i], PORT_buff[i]);
				sprintf(send_buffer, "[Server] Someone comes.\n");
				for(j = 0; j <= maxi; j++)
					if(client_fd[i] >= 0 && i != j)
						write(client_fd[j], send_buffer, sizeof(send_buffer));
					else if(i == j)
						write(client_fd[j], You, sizeof(You));
			}
			if(--nready == 0)
				continue;

			/********************INITIALIZE A CLIENT********************/
		}
		for(i = 0; i <= maxi; i++){
			memset(receive_buffer, 0, sizeof(receive_buffer));
			memset(send_buffer, 0, sizeof(send_buffer));
			memset(You, 0, sizeof(You));
			if(client_fd[i] < 0)
				continue;
			if(FD_ISSET(client_fd[i], &ready_fdset)){
				if(read(client_fd[i], receive_buffer, sizeof(receive_buffer)) == 0)
					goto Exit;

				strcpy(token_buffer, receive_buffer);
				temp = strtok(token_buffer, " ");
				
				/********************EXIT********************/
				if(strcmp(temp, "exit\n") == 0 || strcmp(temp, "exit") == 0 || strlen(receive_buffer) == 0){
					Exit:
						sprintf(send_buffer, "[Server] %s is offine.\n", client_nick[i]);
						for(j = 0; j <= maxi; j++)
							if(client_fd[i] >= 0 && i != j)
								write(client_fd[j], send_buffer, sizeof(send_buffer));
						close(client_fd[i]);
						FD_CLR(client_fd[i], &client_fdset);
						client_fd[i] = -1;
						while(client_fd[maxi] == -1){
							--maxi;
							if(maxi == -1)
								break;
						}
						memset(client_nick[i], 0, sizeof(client_nick[i]));
						memset(PORT_buff[i], 0, sizeof(PORT_buff[i]));
						fprintf(stderr, "[Server] Connection to %s closed!\n", IP_buff[i]);
						bzero(&IP_buff[i], sizeof(IP_buff[i]));
				}

				/********************EXIT********************/

				/********************NAME********************/
				else if(strcmp(temp, "name\n") == 0 || strcmp(temp, "name") == 0){
					if(temp = strtok(NULL, " ")){
						if(strlen(temp) > 13){
							sprintf(You, "[Server] ERROR : Your nickname should not be longer than 12 characters!\n");
							write(client_fd[i], You, sizeof(You));
							goto Next;
						}
						else if(strlen(temp) < 3){
							sprintf(You, "[Server] ERROR : Your nickname should not be shorter than 2 characters!\n");
							write(client_fd[i], You, sizeof(You));
							goto Next;
						}
						if(strcmp(temp, "anonymous\n") == 0 || strcmp(temp, "anonymous") == 0){
							sprintf(You, "[Server] ERROR : Your nickname should not be anonymous!\n");
							write(client_fd[i], You, sizeof(You));
							goto Next;
						}	
						for(j = 0; j <= sizeof(client_nick[i]); j++)
							old_nick[j] = client_nick[i][j];
						memset(client_nick[i], 0, sizeof(client_nick[i]));
						sprintf(client_nick[i], "%s", temp);
						if(client_nick[i][strlen(client_nick[i])-1] == '\n')
							client_nick[i][strlen(client_nick[i])-1] = '\0';
						for(j = 0; j <= maxi; j++)
							if((strcmp(client_nick[i], client_nick[j]) == 0) && i != j){
								sprintf(You, "[Server] ERROR : This name has been used!\n");
								memset(client_nick[i], 0, sizeof(client_nick[i]));
								sprintf(client_nick[i], old_nick);
								memset(old_nick, 0, sizeof(old_nick));
								write(client_fd[i], You, sizeof(You));
								goto Next;
							}
						sprintf(You, "[Server] You are now known as %s\n", client_nick[i]);
						sprintf(send_buffer, "[Server] %s is now known as %s\n", old_nick, client_nick[i]);
						for(j = 0; j <= maxi; j++)
							if(client_fd[i] >= 0 && i != j)
								write(client_fd[j], send_buffer, sizeof(send_buffer));
							else if(i == j)
								write(client_fd[j], You, sizeof(You));
						memset(old_nick, 0, sizeof(old_nick));
					}
					else{
						sprintf(You, "[Server] ERROR : You should input a new name!\n");
						write(client_fd[i], You, sizeof(You));
					}
				}

				/********************NAME********************/
				
				/********************WHO********************/
				else if(strcmp(temp, "who\n") == 0 || strcmp(temp, "who") == 0){
					for(j = 0; j <= maxi; j++){
						if(client_fd[j] >= 0){
							sprintf(You, "[Server] %s from %s/%s", client_nick[j], IP_buff[j], PORT_buff[j]);
							if(i == j)
								strcat(You, "! <-- me\n");
							else
								strcat(You, "!\n");
							write(client_fd[i], You, sizeof(You));
							memset(You, 0, sizeof(You));
						}
					}
				}

				/********************WHO********************/

				/********************YELL********************/
				else if(strcmp(temp, "yell\n") == 0 || strcmp(temp, "yell") == 0){
					if(temp = strtok(NULL, " ")){
						sprintf(You, "[Server]( You ) yelled : ");
						sprintf(send_buffer, "[Server] ( %s ) yelled : ", client_nick[i]);
						do{
							strcat(You, " ");
							strcat(send_buffer, " ");
							strcat(You, temp);
							strcat(send_buffer, temp);
						}while(temp = strtok(NULL, " "));
						for(j = 0; j <= maxi; j++)
							if(client_fd[i] >= 0 && i != j)
								write(client_fd[j], send_buffer, sizeof(send_buffer));
							else if(i == j)
								write(client_fd[j], You, sizeof(You));
					}
					else{
						sprintf(You, "[Server] ERROR : You should input something you want to say!\n");
						write(client_fd[i], You, sizeof(You));
					}
				}

				/********************YELL********************/

				/********************TELL********************/
				else if((strcmp(temp, "tell\n") == 0) || (strcmp(temp, "tell") == 0)){
					if(temp = strtok(NULL, " ")){
						if(strcmp(client_nick[i], "anonymous") == 0)
							sprintf(You, "[Server] ERROR : You can not use tell command, because you are anonymous.\n");
						else if(strcmp(temp, "anonymous") == 0)
							sprintf(You, "[Server] ERROR : You can not use tell command to talk to an anonymous.\n");
						else{
							for(j = 0; j <= maxi; j++)
								if(strcmp(temp, client_nick[j]) == 0){
									sprintf(send_buffer, "[Server] ( %s ) told you : ", client_nick[i]);
									while(temp = strtok(NULL, " ")){
										strcat(send_buffer, " ");
										strcat(send_buffer, temp);
									}
									write(client_fd[j], send_buffer, sizeof(send_buffer));
									sprintf(You, "[Server] SUCCESS : Successfully send the message to %s!\n", client_nick[j]);
									write(client_fd[i], You, sizeof(You));
									goto Next;
								}
							sprintf(You, "[Server] ERROR : Can't find %s. Please use who to check again!\n", temp);
						}
						write(client_fd[i], You, sizeof(You));
					}
					else{
						sprintf(You, "[Server] ERROR : You should input a user's name and something you want to say!\n");
						write(client_fd[i], You, sizeof(You));
					}
				}

				/********************TELL********************/

				/********************HELP********************/
				else if(strcmp(temp, "help\n") == 0 || strcmp(temp, "help") == 0){
					sprintf(You, "[Server] Here are the command in this chatroom :\nexit                      --- disconnect from server.\nname <NEWNAME>            --- rename.\nwho                       --- use to check online users.\nyell <MESSAGE>            --- say something to everyone.\ntell <USERNAME> <MESSAGE> --- say something to someone.\n");
					write(client_fd[i], You, sizeof(You));
				}

				/********************HELP********************/

				/********************ERROR COMMAND********************/
				else{
					sprintf(You, "[Server] ERROR : Invalid command %s", temp);
					if(You[strlen(You)-1] == '\n')
						You[strlen(You)-1] = '\0';
					strcat(You, "! You can use help to check for the usage command!\n");
					write(client_fd[i], You, sizeof(You));
				}

				/********************ERROR COMMAND********************/
				Next:
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
