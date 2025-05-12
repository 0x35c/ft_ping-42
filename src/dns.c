#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>

int dns_lookup(char *ip_addr, char *hostname, struct sockaddr_in *addr_con)
{
	struct hostent *host = gethostbyname(hostname);
	if (!host) {
		dprintf(2, "Hostname %s doesn't exist or has invalid format.",
		        hostname);
		return 1;
	}
	strcpy(ip_addr, inet_ntoa(*(struct in_addr *)host->h_addr));
	(*addr_con).sin_family = host->h_addrtype;
	(*addr_con).sin_port = htons(0);
	(*addr_con).sin_addr.s_addr = *(long *)host->h_addr;

	return 0;
}
