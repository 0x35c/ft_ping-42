#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>

#include "ft_ping.h"

int main(int ac, char **av)
{
	struct sockaddr_in addr_con;
	int ttl_val = 48, packetsize = 56;
	struct timeval tv_out;
	struct option_lst *options = NULL;
	struct stats stats;
	// char *hostname = av[1];
	char *hostname = NULL;

	if (ac < 2) {
		dprintf(2, "Usage: ./ft_ping [option ...] host ...\n");
		return 1;
	}

	if (parse_options(ac, av, &hostname, &options) < 0)
		return 1;
	ttl_val = get_option_arg(options, FL_TTL);
	if (!ttl_val)
		ttl_val = 48;
	packetsize = get_option_arg(options, FL_SIZE);
	if (!packetsize)
		packetsize = 56;

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
		return err("Setting socket for IP protocol to FL_TTL failed!");

	tv_out.tv_sec = 1;
	tv_out.tv_usec = 0;
	if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&tv_out,
	               sizeof(tv_out)))
		return err("Setting socket timeout for receiving failed!");

	ping(sockfd, &addr_con, options, &stats, hostname);

	for (struct option_lst *it = options; it;) {
		struct option_lst *tmp = it;
		it = it->next;
		free(tmp);
	}
	close(sockfd);

	return 0;
}
