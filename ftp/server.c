#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <sys/types.h>
#include <dirent.h>  /* opendir readdir ... */

#include <sys/stat.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>

#include <ncurses.h>

#include "gettime.h"
#include "def_value.h"
#include "filequeue.h"

int create_socket(int* socket_fd, struct sockaddr_in* srv, char* server_IP, int server_port);
void server(int socket_fd, int* client_fd, struct sockaddr_in* cli, socklen_t addrlen);

int main(int argc, char** argv)
{
	int socket_fd, client_fd[NUMOFCLIENT];
	struct sockaddr_in srv, cli;
	socklen_t addrlen = sizeof(cli);

	if(argc != 2){
		fprintf(stderr, "Usage: ./server port\n");
		return -1;
	}
	
	if(create_socket(&socket_fd, &srv, NULL, atoi(argv[1])) == -1)
		return -1;
	
	server(socket_fd, client_fd, &cli, addrlen);
	return 0;
}

int create_socket(int* socket_fd, struct sockaddr_in* srv, char* server_IP, int server_port)
{
	int Rcvbuf, Sndbuf;

	if((*socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
		fprintf(stderr, "[SERVER] create socket error\n");
		return -1;
	}
	
	bzero(srv, sizeof(*srv));
	srv->sin_family = AF_INET;
	srv->sin_port = htons(server_port);
	srv->sin_addr.s_addr = inet_addr("0.0.0.0");

	if(bind(*socket_fd, (struct sockaddr*)srv, sizeof(*srv)) == -1){
		fprintf(stderr, "[SERVER] bind error\n");
		return -1;
	}

	if(listen(*socket_fd, 20) == -1){
		fprintf(stderr, "[SERVER] listen error\n");
		return -1;
	}

	Rcvbuf = RECV_BUFF;
	Sndbuf = SEND_BUFF;
	setsockopt(*socket_fd, SOL_SOCKET, SO_RCVBUF, (const char*)&Rcvbuf, sizeof(int));
	setsockopt(*socket_fd, SOL_SOCKET, SO_SNDBUF, (const char*)&Sndbuf, sizeof(int));

	return 0;
}

void server(int socket_fd, int* client_fd, struct sockaddr_in* cli, socklen_t addrlen)
{
	int Rcvbuf, Sndbuf, maxfd, nready, stdineof[NUMOFCLIENT], val, ofile[NUMOFCLIENT], ifile[NUMOFCLIENT], i, j, nclient, pnclient, transfering[NUMOFCLIENT], synchronizing[NUMOFCLIENT], done[NUMOFCLIENT], filelength[2][NUMOFCLIENT];
	//  0 : download; 1 : upload;
	ssize_t nread, nwritten;
	fd_set read_set, write_set;
	char to[NUMOFCLIENT][MAXLINE], fr[NUMOFCLIENT][MAXLINE], filename[i][MAXLINE], nickname[NUMOFCLIENT][LENOFNICK], Sfilename[MAXLINE];
	char *to_inptr[NUMOFCLIENT], *to_outptr[NUMOFCLIENT], *fr_inptr[NUMOFCLIENT], *fr_outptr[NUMOFCLIENT], *temp;
	FILE* fptr[NUMOFCLIENT];

	struct stat data;
	struct dirent* direntp[NUMOFCLIENT];
	DIR* dirp[NUMOFCLIENT];
	struct filequeue FQ[NUMOFCLIENT];
	struct filenode* FN;

	nclient = pnclient = 0;
	maxfd = socket_fd;
	memset(Sfilename, 0, MAXLINE);
	for(i = 0; i < NUMOFCLIENT; i++){
		client_fd[i] = ofile[i] = ifile[i] = filelength[0][i] = filelength[1][i] = -1;
		stdineof[i] = transfering[i] = synchronizing[i] = 0;
		done[i] = 1;
		to_inptr[i] = to_outptr[i] = to[i];
		fr_inptr[i] = fr_outptr[i] = fr[i];
		fptr[i] = NULL;
		memset(to[i], 0, MAXLINE);
		memset(fr[i], 0, MAXLINE);
		memset(nickname[i], 0, LENOFNICK);
		memset(filename[i], 0, MAXLINE);
		FQ[i].start = NULL;
		FQ[i].end = NULL;
		FQ[i].nfile = 0;
	}

	while(1){
		FD_ZERO(&read_set);
		FD_ZERO(&write_set);
		
		nclient = pnclient;
		FD_SET(socket_fd, &read_set);
		for(i = 0; i < nclient; i++){
//			if((to_inptr[i] < &to[i][MAXLINE]) && (ifile[i] >= 0) && (transfering[i] != 1))
			if((to_inptr[i] < &to[i][MAXLINE]) && (ifile[i] >= 0))
				FD_SET(ifile[i], &read_set);
//			if((fr_inptr[i] < &fr[i][MAXLINE]) && (client_fd[i] >= 0) && (synchronizing[i] != 1))
			if((fr_inptr[i] < &fr[i][MAXLINE]) && (client_fd[i] >= 0))
				FD_SET(client_fd[i], &read_set);
			if((to_outptr[i] != to_inptr[i]) && (client_fd[i] >= 0))  /* there is something in the buffer */
				FD_SET(client_fd[i], &write_set);
			if((fr_outptr[i] != fr_inptr[i]) && (ofile[i] >= 0))  /* there is something in the buffer */
				FD_SET(ofile[i], &write_set);

			maxfd = (maxfd > ifile[i]) ? maxfd : ifile[i];
			maxfd = (maxfd > ofile[i]) ? maxfd : ofile[i];
		}
		
//		fprintf(stderr, "ready\n");
		nready = select(maxfd + 1, &read_set, &write_set, NULL, NULL);
//		fprintf(stderr, "go\n");
		if(FD_ISSET(socket_fd, &read_set)){
			for(i = 0; i < NUMOFCLIENT; i++){
				if(client_fd[i] == -1){
					if((client_fd[i] = accept(socket_fd, (struct sockaddr*)cli, &addrlen)) == -1){
						fprintf(stderr, "[SERVER] accept new client error\n");
						return;
					}
					fprintf(stderr, "[SERVER] new client comes\n");
					break;
				}
			}
			
			if(i != NUMOFCLIENT)
				pnclient = (nclient > i) ? nclient : i+1;
			else
				fprintf(stderr, "[SERVER] number of clients reached limits\n");
			maxfd = (maxfd > client_fd[i]) ? maxfd : client_fd[i];
		
			Rcvbuf = RECV_BUFF;
			Sndbuf = SEND_BUFF;
			setsockopt(client_fd[i], SOL_SOCKET, SO_RCVBUF, (const char*)&Rcvbuf, sizeof(int));
			setsockopt(client_fd[i], SOL_SOCKET, SO_SNDBUF, (const char*)&Sndbuf, sizeof(int));

			if(--nready == 0)
				continue;
		}
		for(i = 0; i < nclient; i++){
			if(client_fd[i] < 0)
				continue;
			if(FD_ISSET(client_fd[i], &read_set)){
				if(transfering[i]){
					if((nread = read(client_fd[i], fr_inptr[i], &fr[i][MAXLINE] - fr_inptr[i])) < 0){
						if(errno != EWOULDBLOCK)
							fprintf(stderr, "[SERVER] read error on socket\n");
	
						if(--nready == 0)
							break;
					}
					else if(nread == 0){
						fprintf(stderr, "[SERVER] %s: client unexpectedly disconnected\n", get_time());
						
						val = fcntl(ofile[i], F_GETFL, 0);
						fcntl(ofile[i], F_SETFL, val | ~O_NONBLOCK);
						val = fcntl(client_fd[i], F_GETFL, 0);
						fcntl(client_fd[i], F_SETFL, val | ~O_NONBLOCK);

						client_fd[i] = ofile[i] = ifile[i] = filelength[0][i] = filelength[1][i] = -1;
						stdineof[i] = transfering[i] = synchronizing[i] = 0;
						done[i] = 1;
						to_inptr[i] = to_outptr[i] = to[i];
						fr_inptr[i] = fr_outptr[i] = fr[i];
						memset(to[i], 0, MAXLINE);
						memset(fr[i], 0, MAXLINE);
						memset(nickname[i], 0, LENOFNICK);
						memset(filename[i], 0, MAXLINE);

						fclose(fptr[i]);
						fptr[i] = NULL;

						while(nfile(&FQ[i]) > 0)
							deletenode(pop(&FQ[i]));
						continue;
					}
					else{
//						fprintf(stderr, "[SERVER] %s: read %d bytes from socket\n", get_time(), nread);
						filelength[0][i] -= nread;
						fr_inptr[i] += nread;
						FD_SET(ofile[i], &write_set);
					}
				}
				else{
					if((nread = read(client_fd[i], fr_inptr[i], MAXLINE)) < 0)
						fprintf(stderr, "[SERVER] read error on socket\n");
					else{
						if(nread == 0){
							fprintf(stderr, "[SERVER] %s: client disconnected\n", get_time());
				
							val = fcntl(client_fd[i], F_GETFL, 0);
							fcntl(client_fd[i], F_SETFL, val | ~O_NONBLOCK);
		
							client_fd[i] = ofile[i] = ifile[i] = filelength[0][i] = filelength[1][i] = -1;
							stdineof[i] = transfering[i] = synchronizing[i] = 0;
							done[i] = 1;
							to_inptr[i] = to_outptr[i] = to[i];
							fr_inptr[i] = fr_outptr[i] = fr[i];
							fptr[i] = NULL;
							memset(to[i], 0, MAXLINE);
							memset(fr[i], 0, MAXLINE);
							memset(nickname[i], 0, LENOFNICK);
							memset(filename[i], 0, MAXLINE);
							
							while(nfile(&FQ[i]) > 0)
								deletenode(pop(&FQ[i]));
							continue;
						}
						temp = strtok(fr_inptr[i], " ");
						if(temp != NULL){
							if((strcmp(temp, "/exit") == 0) || (strcmp(temp, "/exit\n") == 0)){
								fprintf(stderr, "[SERVER] %s: client disconnected\n", get_time());
					
								val = fcntl(client_fd[i], F_GETFL, 0);
								fcntl(client_fd[i], F_SETFL, val | ~O_NONBLOCK);
		
								client_fd[i] = ofile[i] = ifile[i] = filelength[0][i] = filelength[1][i] = -1;
								stdineof[i] = transfering[i] = synchronizing[i] = 0;
								done[i] = 1;
								to_inptr[i] = to_outptr[i] = to[i];
								fr_inptr[i] = fr_outptr[i] = fr[i];
								fptr[i] = NULL;
								memset(to[i], 0, MAXLINE);
								memset(fr[i], 0, MAXLINE);
								memset(nickname[i], 0, LENOFNICK);
								memset(filename[i], 0, MAXLINE);
								
								while(nfile(&FQ[i]) > 0)
									deletenode(pop(&FQ[i]));
								continue;
							}
							else{
								if((strcmp(temp, "/put") == 0) && (strlen(nickname[i]) != 0)){
									temp = strtok(NULL, " ");
								
									sprintf(filename[i], "%s/%s", nickname[i], temp);
									fptr[i] = fopen(filename[i], "w");
									ofile[i] = fileno(fptr[i]);
									temp = strtok(NULL, " ");
									filelength[0][i] = atoi(temp);

									val = fcntl(client_fd[i], F_GETFL, 0);
									fcntl(client_fd[i], F_SETFL, val | O_NONBLOCK);
									val = fcntl(ofile[i], F_GETFL, 0);
									fcntl(ofile[i], F_SETFL, val | O_NONBLOCK);
			
									transfering[i] = 1;
									done[i] = 0;
								}
								else if(strcmp(temp, "/nick") == 0){
									temp = strtok(NULL, " ");

									if(stat(temp, &data) != 0){
										if(mkdir(temp, S_IRWXU) != 0){
											fprintf(stderr, "[SERVER] create directory failed\n");
										}
										fprintf(stderr, "[SERVER] directory created\n");
									}
									else{
										fprintf(stderr, "[SERVER] directory already exist\n");

										memset(nickname[i], 0, LENOFNICK);
										sprintf(nickname[i], "%s", temp);
										sprintf(to_inptr[i], "[SERVER] Welcome to dropbox-like server! : %s", nickname[i]);
										write(client_fd[i], to_outptr[i], MAXLINE);

										if((dirp[i] = opendir(nickname[i])) != NULL){
											while((direntp[i] = readdir(dirp[i])) != NULL){
												sprintf(Sfilename, "%s/%s", nickname[i], direntp[i]->d_name);
												if(stat(Sfilename, &data) != -1)
													for(j = 0; j < nclient; j++)
														if(strcmp(filename[j], Sfilename) == 0)
															if(transfering[j] == 1)
																break;
													if((!S_ISDIR(data.st_mode)) && (j == nclient))
														push(&FQ[i], createnode(Sfilename));
											}
											closedir(dirp[i]);
										}
									}
								}
								else if((strcmp(temp, "/next") == 0) || (strcmp(temp, "/next\n") == 0)){
									done[i] = 1;
								}
								else{
									//write(client_fd[i], "No such command", MAXLINE);
								}
							}
						}
						memset(fr[i], 0, MAXLINE);
						memset(to[i], 0, MAXLINE);
					}

					if(--nready == 0)
						break;
				}
			}
			if(ofile[i] >= 0){
				if(FD_ISSET(ofile[i], &write_set) && ((nread = fr_inptr[i] - fr_outptr[i]) > 0)){
					if((nwritten = write(ofile[i], fr_outptr[i], nread)) < 0){
						if(errno != EWOULDBLOCK)
							fprintf(stderr, "[SERVER] write error to file\n");
					}
					else{
//						fprintf(stderr, "[SERVER] %s: wrote %d bytes to file, %d left\n", get_time(), nwritten, filelength[0][i]);
						fr_outptr[i] += nwritten;
						if(fr_outptr[i] == fr_inptr[i]){
							fr_outptr[i] = fr_inptr[i] = fr[i];
							if(filelength[0][i] == 0){
								fprintf(stderr, "[SERVER] %s: EOF on socket\n", get_time());
								transfering[i] = 0;
								done[i] = 1;

								val = fcntl(client_fd[i], F_GETFL, 0);
								fcntl(client_fd[i], F_SETFL, val | ~O_NONBLOCK);
								val = fcntl(ofile[i], F_GETFL, 0);
								fcntl(ofile[i], F_SETFL, val | ~O_NONBLOCK);
	
								fclose(fptr[i]);
								filelength[0][i] = ofile[i] = -1;

								for(j = 0; j < nclient; j++){
									if((strcmp(nickname[i], nickname[j]) == 0) && (i != j)){
										push(&FQ[j], createnode(filename[i]));
									}
								}
								memset(filename[i], 0, MAXLINE);
							}
							memset(fr[i], 0, MAXLINE);
						}
					}

					if(--nready == 0)
						break;
				}
			}


			if(ifile[i] < 0)
				continue;
			if(FD_ISSET(ifile[i], &read_set)){
				if(synchronizing[i]){
					if((nread = read(ifile[i], to_inptr[i], &to[i][MAXLINE] - to_inptr[i])) < 0){
						if(errno != EWOULDBLOCK)
							fprintf(stderr, "[SERVER] %s: read error on file\n", get_time());

						if(--nready == 0)
							break;
					}
					else if(nread == 0){
						if(filelength[1][i] > 0)
							fprintf(stderr, "[SERVER] %s: read error on file\n", get_time());
					}
					else{
//						fprintf(stderr, "[SERVER] %s: read %d bytes from file\n", get_time(), nread);
						filelength[1][i] -= nread;
						to_inptr[i] += nread;
						FD_SET(client_fd[i], &write_set);
					}
				}
			}
			if(client_fd[i] >= 0){
				if(FD_ISSET(client_fd[i], &write_set) && ((nread = to_inptr[i] - to_outptr[i]) > 0)){
					if((nwritten = write(client_fd[i], to_outptr[i], nread)) < 0){
						if(errno != EWOULDBLOCK)
							fprintf(stderr, "[SERVER] write error to socket\n");
					}
					else{
//						fprintf(stderr, "[SERVER] %s: wrote %d bytes to socket, %d left\n", get_time(), nwritten, filelength[1][i]);
						to_outptr[i] += nwritten;
						if(to_outptr[i] == to_inptr[i]){
							to_inptr[i] = to_outptr[i] = to[i];
							if(filelength[1][i] == 0){
								fprintf(stderr, "[SERVER] %s: EOF on file\n", get_time());
								fclose(fptr[i]);

								val = fcntl(client_fd[i], F_GETFL, 0);
								fcntl(client_fd[i], F_SETFL, val | ~O_NONBLOCK);
								val = fcntl(ifile[i], F_GETFL, 0);
								fcntl(ifile[i], F_SETFL, val | ~O_NONBLOCK);
	
								ifile[i] = -1;
								synchronizing[i] = 0;
								filelength[1][i] = -1;
								memset(to[i], 0, MAXLINE);
							}
						}
					}

					if(--nready == 0)
						break;
				}
			}
		}
		for(i = 0; i < nclient; i++){
			if(done[i] == 1){
				if((nfile(&FQ[i]) > 0) && (synchronizing[i] != 1) && (transfering[i] != 1)){
					FN = pop(&FQ[i]);
					sprintf(filename[i], "%s", FN->filename);
					deletenode(FN);
					if((fptr[i] = fopen(filename[i], "r")) == NULL){
						fprintf(stderr, "[SERVER] file %s open error\n", filename[i]);
						continue;
					}
					fseek(fptr[i], 0, SEEK_END);
					filelength[1][i] = ftell(fptr[i]);
					fprintf(stderr, "[SERVER] %d is syncing! %s %d\n", i, filename[i], filelength[1][i]);
					if(filelength[1][i] == 0){
						fprintf(stderr, "[SERVER] empty file\n");
						fclose(fptr[i]);
						continue;
					}
					fclose(fptr[i]);
					fptr[i] = fopen(filename[i], "r");
					//fseek(fptr[i], 0, SEEK_SET);  //  why not work??
					//rewind(fptr[i]);  //  why not work??
					ifile[i] = fileno(fptr[i]);
			
					sprintf(to_outptr[i], "/sync %s %d", filename[i], filelength[1][i]);
					write(client_fd[i], to_outptr[i], MAXLINE);
	
					val = fcntl(client_fd[i], F_GETFL, 0);
					fcntl(client_fd[i], F_SETFL, val | O_NONBLOCK);
					val = fcntl(ifile[i], F_GETFL, 0);
					fcntl(ifile[i], F_SETFL, val | O_NONBLOCK);
	
					synchronizing[i] = 1;
					done[i] = 0;
				}
			}
		}
	}
}
