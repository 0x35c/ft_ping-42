#include <netdb.h>
#include <stdio.h>
#include <strings.h>

#include "ft_ping.h"

int main(int ac, char **av)
{
	struct sockaddr_in addr_con;
	int ttl_val = 48; // TODO -t flag
	struct timeval tv_out;
	struct stats stats;

	if (ac < 2) {
		dprintf(2, "Wrong usage: ./ft_ping [option ...] host ...\n");
		return 1;
	}

	bzero(&stats, sizeof(stats));
	if (dns_lookup(stats.ip, av[1], &addr_con))
		return 1;
	if (reverse_dns_lookup(stats.ip, stats.host))
		return 1;
	stats.packetsize = 56;
	stats.ttl = 48;

	int sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if (sockfd < 0)
		return err("Failed to create socket");

	if (setsockopt(sockfd, SOL_IP, IP_TTL, &ttl_val, sizeof(ttl_val)))
		return err("Setting socket for IP protocol to TTL failed!");

	tv_out.tv_sec = 1;
	tv_out.tv_usec = 0;
	if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&tv_out,
	               sizeof(tv_out)))
		return err("Setting socket timeout for receiving failed!");

	ping(sockfd, &addr_con, &stats, av[1]);

	return 0;
}
