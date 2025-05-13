#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>

#include "ft_ping.h"

int main(int ac, char **av)
{
	struct sockaddr_in addr_con;
	int ttl_val = 48, packetsize = 56;
	struct timeval tv_out;
	struct option *options = NULL;
	struct stats stats;
	char *hostname = av[1];

	if (ac < 2) {
		dprintf(2, "Usage: ./ft_ping [option ...] host ...\n");
		return 1;
	}

	if (av[1][0] != '-')
		goto hostname;
	options = parse_options(&av[1], &hostname);
	if (!options)
		return 1;
	ttl_val = get_option_arg(options, TTL);
	if (!ttl_val)
		ttl_val = 48;
	packetsize = get_option_arg(options, SIZE);
	if (!packetsize)
		packetsize = 56;

hostname:
	bzero(&stats, sizeof(stats));
	if (dns_lookup(stats.ip, hostname, &addr_con))
		return 1;
	if (reverse_dns_lookup(stats.ip, stats.host))
		return 1;
	stats.packetsize = packetsize;
	stats.ttl = ttl_val;

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

	ping(sockfd, &addr_con, options, &stats, hostname);

	for (struct option *it = options; it;) {
		struct option *tmp = it;
		it = it->next;
		free(tmp);
	}

	return 0;
}
