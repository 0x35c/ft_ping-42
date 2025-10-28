#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

int dns_lookup(char *ip_addr, char *hostname, struct sockaddr_in *addr_con)
{
	if (!hostname) {
		dprintf(2, "Please input a hostname.\n");
		return -1;
	}

	struct hostent *host = gethostbyname(hostname);
	if (!host) {
		dprintf(2, "Hostname %s doesn't exist or has invalid format.\n",
		        hostname);
		return -1;
	}
	strcpy(ip_addr, inet_ntoa(*(struct in_addr *)host->h_addr));
	(*addr_con).sin_family = host->h_addrtype;
	(*addr_con).sin_port = htons(0);
	(*addr_con).sin_addr.s_addr = *(long *)host->h_addr;

	return 0;
}

int reverse_dns_lookup(char *ip_addr, char *host)
{
	struct sockaddr_in tmp_addr;

	tmp_addr.sin_family = AF_INET;
	tmp_addr.sin_addr.s_addr = inet_addr(ip_addr);
	if (getnameinfo((struct sockaddr *)&tmp_addr,
	                sizeof(struct sockaddr_in), host, NI_MAXHOST, NULL, 0,
	                NI_NAMEREQD)) {
		dprintf(2, "Could not resolve reverse lookup of %s\n", ip_addr);
		return -1;
	}

	return 0;
}
