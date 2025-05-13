#pragma once

#include <netdb.h>
#include <netinet/ip_icmp.h>

// #define assert(X)

typedef enum { COUNT = 1, INTERVAL, QUIET, SIZE, VERBOSE, TTL } e_flag;

struct option {
	e_flag flag;
	int arg;
	struct option *next;
};

struct rtt {
	double min;
	double avg;
	double max;
};

struct stats {
	size_t packetsize;
	char host[NI_MAXHOST];
	char ip[15];
	int req_count;
	int ttl;
	struct rtt rtt;
};

struct option *parse_options(char **options, char **hostname);
int get_option_arg(struct option *options, e_flag flag);
void ping(int sockfd, struct sockaddr_in *addr_con, struct option *options,
          struct stats *stats, char *host_input);
int dns_lookup(char *ip_addr, char *hostname, struct sockaddr_in *addr_con);
int reverse_dns_lookup(char *ip_addr, char *host);
int err(char *str);
unsigned int checksum(void *data, size_t len);
void calculate_rtt(struct stats *stats);
