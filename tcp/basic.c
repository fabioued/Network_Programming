// for npch4-expanded.ppt
#include <sys/socket.h>
/*
int socket(int family, int type, int protocol);
	returns: nonnegative descriptor if OK, -1 on error
	family:
		AF_INET		IPv4
		AF_INET6	IPv6

		SOCK_STREAM
int connect (int sockfd, const struct sockaddr *servaddr, socklen_t addrlen);
	returns: 0 of OK, -1 on error
int bind (int sockfd, const struct sockaddr *myaddr, socklen_t addrlen);
	returns: 0 if OK, -1 on error
int listen (int sockfd, int backlog);
	returns: 0 if OK, -1 on error
int accept (int sockfd, struct sockaddr *cliaddr, socklen_t *addrlen);
	returns: nonnegative descriptor if OK, -1 on error

int getsockname (int sockfd, struct sockaddr *localaddr, socklen_t *addrlen);
	returns: 0 if OK, -1 on error
int getpeername (int sockfd, struct sockaddr *peeraddr, socklen_t *addrlen);
	returns: 0 if OK, -1 on error
*/
#include <unistd.h>
/*
pid_t fork(void);
	returns: 0 in child, process ID of child in parent, -1 on error
int close (int sockfd);
	returns: 0 if OK, -1 on error;
*/


