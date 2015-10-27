char *sock_ntop_host(const struct sockaddr *sa, socklen_t sa_len)
{
	static char str[128];
	struct sockaddr_in *sin = (struct sockaddr_in *) sa;
	
	if( sa->sa_family != AF_INET )
		//error_quit("sock_ntop_host: the type must be AF_INET");
		fprintf(stderr, "sock_ntop_host: the type must be AF_INET\n");

	if (inet_ntop(AF_INET, &sin->sin_addr, str, sizeof(str)) == NULL)
		//error_quit("inet_ntop error");
		fprintf(stderr, "inet_ntop error\n");

	return str;
}
