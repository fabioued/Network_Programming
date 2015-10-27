void send_v4(void)
{
	int len;
	struct icmp *icmp;
	icmp = (struct icmp *)sendbuf;
	icmp->icmp_type = ICMP_ECHO;
	icmp->icmp_code = 0;
	icmp->icmp_seq = nsent++;
	memset(icmp->icmp_data, 0xa5, datalen);	/* fill in with pattern */

	gettimeofday((struct timeval *)icmp->icmp_data, NULL);

	len = 8 + datalen;	/* checksum for header and data */
	icmp->icmp_cksum = 0;
	icmp->icmp_cksum = in_cksum((u_short *)icmp, len);

	sendto(sock_fd, sendbuf, len, 0, pr->sa_send, pr->sa_len);
}
