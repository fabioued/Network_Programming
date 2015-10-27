#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <netinet/in_systm.h>
#include <netinet/ip.h>		/* IP variable */
#include <netinet/ip_icmp.h>

#include <arpa/inet.h>	/* inet_ntop() */
#include <signal.h>	/* SIGNAL_ALRM */
#include <unistd.h>	/* getopt(), opterr... */
#include <netdb.h>	/* struct addrinfo */
#include <errno.h>
#include <sys/socket.h>	/* struct msghdr */

#define BUFSIZE 1500

#ifdef IPV6

#include <netinet/ip6.h>
#include <netinet/icmp6.h>

#endif

char sendbuf[BUFSIZE];

int datalen;	/* bytes of data following ICMP header */
char *hostname;
int nsent;	/* add 1 for each sendto() */
pid_t pid;	/* our PID */
int sock_fd;
int verbose;	/* number of attribute */

void init_v6(void);
void proc_v4(char *, ssize_t, struct msghdr *, struct timeval *);
void proc_v6(char *, ssize_t, struct msghdr *, struct timeval *);
void send_v4(void);

void send_v6(void);
void readloop(void);
void sig_alrm(int);
void tv_sub(struct timeval *, struct timeval *);	/* RTT of local to remote, and remote to local */

struct proto
{
	void (*fproc) (char *, ssize_t, struct msghdr *, struct timeval *);
	void (*fsend) (void);
	void (*finit) (void);
	struct sockaddr *sa_send;	/* sockaddr{} for send, from getaddrinfo */
	struct sockaddr *sa_recv;	/* sockaddr{} for recv */
	socklen_t sa_len;	/* length of sockaddr {}s */
	int icmpproto;	/* IPPROTO_xxx value for ICMP ?????*/
} *pr;

//struct addrinfo *host_serv(const char *host, const char *serv, int family, int socktype);	/* http://m.blog.csdn.net/blog/AAA20090987/8594797 */
//char *sock_ntop_host(const struct sockaddr *sa, socklen_t sa_len);
