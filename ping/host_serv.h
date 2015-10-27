struct addrinfo *host_serv(const char *host, const char *serv, int family, int socktype)
{
	int n;
	struct addrinfo hints, *res;

	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_flags = AI_CANONNAME;
	hints.ai_family = family; 
	hints.ai_socktype = socktype;

	n = getaddrinfo(host, serv, &hints, &res);
	if ( n != 0 )
		//error_quit("getaddrinfo error");
		fprintf(stderr, "getaddrinfo error\n");

	return res;
}
