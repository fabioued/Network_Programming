#include "ping.h"
#include "sock_ntop_host.h"	/* XX */
#include "host_serv.h"	/* XX */
#include "tv_sub.h"
#include "in_cksum.h"
#include "readloop.h"
#include "sig_alrm.h"
#include "proc_v4.h"
#include "send_v4.h"


struct proto proto_v4 = {proc_v4, send_v4, NULL, NULL, NULL, 0, IPPROTO_ICMP};	/* initialize proto_v4 with predefined function and assign value to variable */

#ifdef IPV6

struct proto proto_v6 = {proc_v6, send_v6, NULL, NULL, NULL, 0, IPPROTO_ICMP};	/* same as above */

#endif

int datalen = 56;	/* data goes with icmp request; */

int main(int argc, char** argv)
{
	int c;	/* option character from file attribute argc, argv */
	struct addrinfo *ai;
	char *h;

	opterr = 0;	/* if you set this variable to zero, getopt does not print any messages, but it still returns the character ? to indicate an error. man page: http://www.delorie.com/gnu/docs/glibc/libc_525.html */
	while((c = getopt(argc, argv, "v")) != -1){
		switch(c){
			case 'v':
				verbose++;
				break;
			case '?':
				//err_quit("unrecognized option: %c", c);
				fprintf(stderr, "unrecognized option: %c\n", c);
				return 0;
		}
	}

	if(optind != argc - 1){	/* Once getopt has found all of the option arguments, you can use this variable to determine where the remaining non-option arguments begin */
		//err_quit("usage: ping [ -v ] <hostname>");
		fprintf(stderr, "usage: ping [ -v ] <hostname>\n");
		return 0;
	}
	hostname = argv[optind];

	pid = getpid() & 0xffff;
	signal(SIGALRM, sig_alrm);

	ai = host_serv(hostname, NULL, 0, 0);

	h = sock_ntop_host(ai->ai_addr, ai->ai_addrlen);
	fprintf(stdout, "PING %s (%s): %d data bytes\n", ai->ai_canonname ? ai->ai_canonname : h, h, datalen);

	if(ai->ai_family == AF_INET){
		pr = &proto_v4;
#ifdef IPV6
	}
	else if(ai->ai_family == AF_INET6){
		pr = &proto_v6;
		if(IN6_IS_ADDR_V4MAPPED(&(((struct sockaddr_in6 *)ai->ai_addr)->sin6_addr))){	/* ????? */
			//err_quit("cannot ping IPv4-mapped IPv6 address");
			fprintf(stderr, "cannot ping IPv4-mapped IPv6 address\n");
		}
#endif
	}
	else{
		//err_quit("unknown address family %d", ai->ai_family);
		fprintf(stderr, "unknown address family %d\n", ai->ai_family);
		return 0;
	}
	
	pr->sa_send = ai->ai_addr;
	pr->sa_recv = calloc(1, ai->ai_addrlen);
	pr->sa_len = ai->ai_addrlen;

	readloop();

	return 0;
}
