void readloop(void)
{
	int size;
	char recvbuf[BUFSIZE], controlbuf[BUFSIZE];
	struct msghdr msg;
	struct iovec iov;
	ssize_t n;
	struct timeval tval;

	if((sock_fd = socket(pr->sa_send->sa_family, SOCK_RAW, pr->icmpproto)) == -1){
		fprintf(stderr, "raw socket create error\n");
		return;
	}
	setuid(getuid());
	if(pr->finit)
		(*pr->finit)();

	size = 60 * 1024;
	setsockopt(sock_fd, SOL_SOCKET, SO_RCVBUF, &size, sizeof(size));
	sig_alrm(SIGALRM);

	iov.iov_base = recvbuf;
	iov.iov_len = sizeof(recvbuf);
	msg.msg_name = pr->sa_recv;
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;
	msg.msg_control = controlbuf;

	while(1){
		msg.msg_namelen = pr->sa_len;
		msg.msg_controllen = sizeof(controlbuf);
		n = recvmsg(sock_fd, &msg, 0);
		if(n < 0){
			if(errno == EINTR)
				continue;
			else
				//err_sys("recvmsg error");
				fprintf(stderr, "recvmsg error %d\n", sock_fd);
		}
	}
	gettimeofday(&tval, NULL);
	(*pr->fproc) (recvbuf, n, &msg, &tval);
}
