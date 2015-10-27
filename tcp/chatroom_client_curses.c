#include "client.h"
#include <locale.h>
#include <ncurses.h>  /* http://invisible-island.net/ncurses/man/ncurses.3x.html */
int main(int argc, char** argv){
	int socket_fd, connect_fd;
	char* temp;
	char IP_buff[16], receive_buffer[800], send_buffer[800], screen_buffer[800];
	memset(receive_buffer, 0, sizeof(receive_buffer));
	memset(send_buffer, 0, sizeof(send_buffer));
	memset(screen_buffer, 0, sizeof(screen_buffer));
	struct sockaddr_in srv;
	if(argc != 3){
		fprintf(stderr, "[Client] Usage: chatroom_client <server_ip> <server_port>\n");
		return 0;
	}


	int port = atoi(argv[2]);
	if(client_setup(&socket_fd, &srv, argv[1], port) == -1)
		return -1;
	

	setlocale(LC_ALL, "en_US.UTF-8");
	initscr();  /* curses.h */
	refresh();

	WINDOW* win[2];
	int xo = 0, xi = 0, yo = 0, yi = LINES - 2;
	win[0] = newwin(yi, COLS, yo, xo);
	win[1] = newwin(1, COLS, yi, xo);
	scrollok(win[0], TRUE);

	inet_ntop(AF_INET, &srv.sin_addr.s_addr, IP_buff, sizeof(IP_buff));
	connect_fd = connect(socket_fd, (struct sockaddr*)&srv, sizeof(srv));
	if(connect_fd == -1){
		sprintf(screen_buffer, "[Client] ERROR : Can't connect to server! connect_fd == %d\n", connect_fd);
		mvwaddstr(win[0], yo, xo, screen_buffer);
		wrefresh(win[0]);
		return -1;
	}
	else{
		sprintf(screen_buffer, "[Client] SUCCESS : Successfully connect to server %s!\n", IP_buff);
		mvwaddstr(win[0], yo, xo, screen_buffer);
		wrefresh(win[0]);
	}
	fd_set client_fdset;
	FD_ZERO(&client_fdset);
	getyx(win[0], yo, xo);
	move(yi, xi);
	wrefresh(win[1]);
	while(1){
		FD_SET(fileno(stdin), &client_fdset);
		FD_SET(socket_fd, &client_fdset);
		select(socket_fd + 1, &client_fdset, NULL, NULL, NULL);
		if(FD_ISSET(fileno(stdin), &client_fdset)){
			wgetstr(win[1], send_buffer);
			if(strcmp(send_buffer, "clear") == 0){
				wclear(win[0]);
				xo = 0;
				yo = 0;
			}
			else{
				strcat(send_buffer, "\n");
				write(socket_fd, send_buffer, sizeof(send_buffer));
			}
			temp = strtok(send_buffer, " ");
			if(strcmp(temp, "exit\n") == 0 || strcmp(temp, "exit") == 0){  /* locally terminate */
				sprintf(screen_buffer, "[Client] Connection close!\n");
				mvwaddstr(win[0], yo, xo, screen_buffer);
				wrefresh(win[0]);
				FD_CLR(socket_fd, &client_fdset);
				break;
			}
			wclear(win[1]);
			wrefresh(win[0]);
			wrefresh(win[1]);
		}
		if(FD_ISSET(socket_fd, &client_fdset)){
			if((read(socket_fd, receive_buffer, sizeof(receive_buffer))) == 0){  /* remotely terminate */
//				sprintf(screen_buffer, "[Client] Connection closed by server %s!\n", IP_buff);
//				mvwaddstr(win[0], yo, xo, screen_buffer);
//				wrefresh(win[0]);
//				FD_CLR(socket_fd, &client_fdset);
//				break;
			}
			sprintf(screen_buffer, "%s", receive_buffer);
			mvwaddstr(win[0], yo, xo, screen_buffer);
			getyx(win[0], yo, xo);
		}
		memset(receive_buffer, 0, sizeof(receive_buffer));
		memset(send_buffer, 0, sizeof(send_buffer));
		memset(screen_buffer, 0, sizeof(screen_buffer));
		
		wrefresh(win[0]);
		move(yi, xi);
		wrefresh(win[1]);
	}
	endwin();  /* curses.h */
	close(socket_fd);
	return 0;
}
