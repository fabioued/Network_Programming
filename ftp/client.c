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

#include "gettime.h"
#include "def_value.h"
#include "filequeue.h"

int create_socket(int* socket_fd, struct sockaddr_in* srv, char* server_IP, int server_port);
void client(int socket_fd, char* nickname);

int main(int argc, char** argv)
{
	int socket_fd;
	FILE* fptr;
	struct sockaddr_in srv;

	if(argc != 4){
		fprintf(stderr, "[CLIENT] Usage: ./client IP port nickname\n");
		return -1;
	}
	
	if(create_socket(&socket_fd, &srv, argv[1], atoi(argv[2])) == -1)
		return -1;
	
	fprintf(stderr, "[CLIENT] connection established\n");

	client(socket_fd, argv[3]);
	return 0;
}

int create_socket(int* socket_fd, struct sockaddr_in* srv, char* server_IP, int server_port)
{
	int Rcvbuf, Sndbuf;

	if((*socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
		fprintf(stderr, "[CLIENT] create socket error\n");
		return -1;
	}
	
	bzero(srv, sizeof(*srv));
	srv->sin_family = AF_INET;
	srv->sin_port = htons(server_port);
	srv->sin_addr.s_addr = inet_addr(server_IP);

	if(connect(*socket_fd, (struct sockaddr*)srv, sizeof(*srv)) == -1){
		fprintf(stderr, "[CLIENT] connect to server error\n");
		return -1;
	}

	Rcvbuf = RECV_BUFF;
	Sndbuf = SEND_BUFF;
	setsockopt(*socket_fd, SOL_SOCKET, SO_RCVBUF, (const char*)&Rcvbuf, sizeof(int));
	setsockopt(*socket_fd, SOL_SOCKET, SO_SNDBUF, (const char*)&Sndbuf, sizeof(int));

	return 0;
}

void client(int socket_fd, char* nickname)
{
	int maxfd, stdineof, val, ifile, ofile, transfering, synchronizing, filelength[2], maxfilelength[2], i, j, sleeptime;
	ssize_t nread, nwritten;
	fd_set read_set, write_set;
	char to[MAXLINE], fr[MAXLINE], filename[MAXLINE], cpy[MAXLINE], scr_buf[MAXLINE];
	char *to_inptr, *to_outptr, *fr_inptr, *fr_outptr, *temp;
	FILE* fptr;
	
	struct stat data;
	struct dirent* direntp;
	DIR* dirp;

	to_inptr = to_outptr = to;
	fr_inptr = fr_outptr = fr;
	ifile = STDIN_FILENO;
	ofile = STDOUT_FILENO;
	transfering = synchronizing = 0;
	filelength[0] = filelength[1] = -1;
	maxfilelength[0] = filelength[1] = -1;
	maxfd = (ifile > socket_fd) ? ifile : socket_fd;
	memset(to, 0, MAXLINE);
	memset(fr, 0, MAXLINE);

	/* Initialize */
	sprintf(to_outptr, "/nick %s", nickname);
	write(socket_fd, to_outptr, MAXLINE);
	read(socket_fd, fr_inptr, MAXLINE);
	fprintf(stderr, "%s\n", fr_inptr);
	memset(to, 0, MAXLINE);

	while(1){
		FD_ZERO(&read_set);
		FD_ZERO(&write_set);
		
		if((to_inptr < &to[MAXLINE]) && (synchronizing != 1))
			FD_SET(ifile, &read_set);
		if((fr_inptr < &fr[MAXLINE]) && (transfering != 1))
			FD_SET(socket_fd, &read_set);
		if(to_outptr != to_inptr)  /* there is something in the buffer */
			FD_SET(socket_fd, &write_set);
		if(fr_outptr != fr_inptr)  /* there is something in the buffer */
			FD_SET(ofile, &write_set);

		maxfd = (ifile > maxfd) ? ifile : maxfd;
		maxfd = (ofile > maxfd) ? ofile : maxfd;
		
		select(maxfd + 1, &read_set, &write_set, NULL, NULL);
		if(FD_ISSET(ifile, &read_set)){
			if(transfering){
				if((nread = read(ifile, to_inptr, &to[MAXLINE] - to_inptr)) < 0){
					if(errno != EWOULDBLOCK){
						fprintf(stderr, "[CLIENT] %s: read error on file\n", get_time());
					}
				}
				else if(nread == 0){
					fprintf(stderr, "[CLIENT] %s: read error on file\n", get_time());
				}
				else{
//					if(nread != MAXLINE){
//						fprintf(stderr, "[CLIENT] %s: read %d bytes from file\n", get_time(), nread);
//					}
					filelength[1] -= nread;
					to_inptr += nread;
					FD_SET(socket_fd, &write_set);
				}
			}
			else{
				if((nread = read(ifile, to_inptr, MAXLINE)) < 0){
					fprintf(stderr, "[CLIENT] %s: read error on file\n", get_time());
				}
				else{
					if(to[strlen(to)-1] == '\n')
						to[strlen(to)-1] = '\0';
					strcpy(cpy, to);
					temp = strtok(cpy, " ");
					if(temp != NULL){
						if((nread == 0) || (strcmp(temp, "/exit") == 0) || (strcmp(temp, "/exit\n") == 0)){
							shutdown(socket_fd, SHUT_WR);  /* FIN signal */
							break;
						}
						else if(strcmp(temp, "/put") == 0){
							temp = strtok(NULL, " ");

							sprintf(filename, "%s", temp);
							if(stat(filename, &data) != 0){
								fprintf(stderr, "[CLIENT] %s: no such file\n", get_time());
							}
							else{
								fprintf(stderr, "[CLIENT] %s: uploading %s\n", get_time(), filename);
								fptr = fopen(filename, "r");
								fseek(fptr, 0, SEEK_END);
								filelength[1] = maxfilelength[1] = ftell(fptr);
								if(filelength[1] == 0){
									fprintf(stderr, "[CLIENT] %s: empty file\n", get_time());
									fclose(fptr);
									continue;
								}
								fclose(fptr);
								fptr = fopen(filename, "r");
								//fseek(fptr, 0, SEEK_SET);  //  why not work??
								//rewind(fptr);  //  why not work??
								ifile = fileno(fptr);
							
								sprintf(to_outptr, "%s %d", to_outptr, filelength[1]);
								write(socket_fd, to_outptr, MAXLINE);
	
								val = fcntl(socket_fd, F_GETFL, 0);
								fcntl(socket_fd, F_SETFL, val | O_NONBLOCK);
								val = fcntl(ifile, F_GETFL, 0);
								fcntl(ifile, F_SETFL, val | O_NONBLOCK);
	
								transfering = 1;
							}
						}
						else if(strcmp(temp, "/nick") == 0){
							temp = strtok(NULL, " ");
							
							/* ...... */

							write(socket_fd, to_outptr, MAXLINE);
						}
						else if(strcmp(temp, "/sleep") == 0){
							temp = strtok(NULL, " ");
							sleeptime = (temp == NULL) ? 20 : atoi(temp);
							fprintf(stderr, "[CLIENT] %s: start to sleep\n", get_time());
							for(i = sleeptime; i > 0; i--){
								fprintf(stderr, "[CLIENT] %s: sleep %d\n", get_time(), i);
								sleep(1);
							}
							fprintf(stderr, "[CLIENT] %s: wake up\n", get_time());
						}
					}
					memset(to, 0, MAXLINE);
					memset(fr, 0, MAXLINE);
				}
			}
		}
		if(FD_ISSET(socket_fd, &write_set) && ((nread = to_inptr - to_outptr) > 0)){
			if((nwritten = write(socket_fd, to_outptr, nread)) < 0){
				if(errno != EWOULDBLOCK){
					fprintf(stderr, "[CLIENT] %s: write error to socket\n", get_time());
				}
			}
			else{
				fprintf(stderr, "\r[CLIENT] %s: process bar [",get_time());
				for(j = 20; j >= 1; j--){
					if((maxfilelength[1] / 20 * j) >= filelength[1])
						fprintf(stderr, "#");
					else
						fprintf(stderr, " ");
				}
				fprintf(stderr, "]");
				to_outptr += nwritten;
				if(to_outptr == to_inptr){
					to_inptr = to_outptr = to;
					if(filelength[1] == 0){
						fprintf(stderr, "\n[CLIENT] %s: upload finished\n", get_time());
						fclose(fptr);
						
						val = fcntl(socket_fd, F_GETFL, 0);
						fcntl(socket_fd, F_SETFL, val | ~O_NONBLOCK);
						val = fcntl(ifile, F_GETFL, 0);
						fcntl(ifile, F_SETFL, val | ~O_NONBLOCK);
	
						ifile = STDIN_FILENO;
						transfering = 0;
						filelength[1] = maxfilelength[1] = -1;
					}
					memset(to, 0, MAXLINE);
				}
			}
		}


		if(FD_ISSET(socket_fd, &read_set)){
			if(synchronizing){
				if((nread = read(socket_fd, fr_inptr, &fr[MAXLINE] - fr_inptr)) < 0){
					if(errno != EWOULDBLOCK){
						fprintf(stderr, "[CLIENT] %s: read error on socket\n", get_time());
					}
				}
				else if(nread == 0){
					fprintf(stderr, "[CLIENT] %s: server unexpectedly disconnected\n", get_time());
					fclose(fptr);
					break;
				}
				else{
//					if(nread != MAXLINE){
//						fprintf(stderr, "[CLIENT] %s: read %d bytes from socket\n", get_time(), nread);
//					}
					filelength[0] -= nread;
					fr_inptr += nread;
					FD_SET(ofile, &write_set);
				}
			}
			else{
				if((nread = read(socket_fd, fr_inptr, MAXLINE)) < 0){
					fprintf(stderr, "[CLIENT] %s: read error on socket\n", get_time());
				}
				else{
					if(nread == 0){
						fprintf(stderr, "[CLIENT] %s: server disconnected\n", get_time());
						break;
					}
					else{
						strcpy(cpy, fr);
						temp = strtok(cpy, " ");
						if(temp != NULL){
							if((strcmp(temp, "/sync") == 0) && (strlen(nickname) != 0)){
								temp = strtok(NULL, " ");
								
								if(stat(nickname, &data) != 0){
									if(mkdir(nickname, S_IRWXU) != 0){
										fprintf(stderr, "[CLIENT] %s: create directory failed\n", get_time());
									}
									fprintf(stderr, "[CLIENT] %s: directory created\n", get_time());
								}
								else{
									fprintf(stderr, "[CLIENT] %s: directory already exist\n", get_time());
								}

								sprintf(filename, "%s", temp);
								fprintf(stderr, "[CLIENT] %s: downloading %s\n", get_time(), filename);
								fptr = fopen(filename, "w");
								ofile = fileno(fptr);
								temp = strtok(NULL, " ");
								filelength[0] = maxfilelength[0] = atoi(temp);

								val = fcntl(socket_fd, F_GETFL, 0);
								fcntl(socket_fd, F_SETFL, val | O_NONBLOCK);
								val = fcntl(ofile, F_GETFL, 0);
								fcntl(ofile, F_SETFL, val | O_NONBLOCK);
			
								synchronizing = 1;
							}
							else{
								fprintf(stderr, "[CLIENT] %s: set your nickname first\n", get_time());
								//write(socket_fd, "No such command", MAXLINE);
							}
						}
					}
					memset(fr, 0, MAXLINE);
				}
			}
		}
		if(FD_ISSET(ofile, &write_set) && ((nread = fr_inptr - fr_outptr) > 0)){
			if((nwritten = write(ofile, fr_outptr, nread)) < 0){
				if(errno != EWOULDBLOCK){
					fprintf(stderr, "[CLIENT] %s: write error to file\n", get_time());
				}
			}
			else{
				fprintf(stderr, "\r[CLIENT] %s: process bar [",get_time());
				for(j = 20; j >= 1; j--){
					if((maxfilelength[0] / 20 * j) >= filelength[0])
						fprintf(stderr, "#");
					else
						fprintf(stderr, " ");
				}
				fprintf(stderr, "]");
//				if(nwritten != MAXLINE){
//					fprintf(stderr, "[CLIENT] %s: wrote %d bytes to file, %d left\n", get_time(), nwritten, filelength[0]);
//				}
				fr_outptr += nwritten;
				if(fr_outptr == fr_inptr){
					fr_outptr = fr_inptr = fr;
					if(filelength[0] == 0){
						fprintf(stderr, "\n[CLIENT] %s: download finished\n", get_time());
						synchronizing = 0;
						fclose(fptr);

						val = fcntl(socket_fd, F_GETFL, 0);
						fcntl(socket_fd, F_SETFL, val | ~O_NONBLOCK);
						val = fcntl(ofile, F_GETFL, 0);
						fcntl(ofile, F_SETFL, val | ~O_NONBLOCK);

						sprintf(to_outptr, "/next");
						write(socket_fd, to_outptr, MAXLINE);

						ofile = STDOUT_FILENO;
						filelength[0] = maxfilelength[0] = -1;
					}
					memset(fr, 0, MAXLINE);
					memset(to, 0, MAXLINE);
				}
			}
		}
	}
	close(socket_fd);
}
