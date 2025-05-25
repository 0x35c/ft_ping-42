#pragma once

#include <netdb.h>
#include <netinet/ip_icmp.h>

// #define assert(X)

typedef enum {
	FL_COUNT = 'c',
	FL_INTERVAL = 'i',
	FL_QUIET = 'q',
	FL_SIZE = 's',
	FL_VERBOSE = 'v',
	FL_TTL = 't',
} e_flag;

struct option_lst {
	e_flag flag;
	int arg;
	struct option_lst *next;
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

int parse_options(int ac, char *const *av, char **hostname,
                  struct option_lst **head);
int get_option_arg(struct option_lst *options, e_flag flag);
void ping(int sockfd, struct sockaddr_in *addr_con, struct option_lst *options,
          struct stats *stats, char *host_input);
int dns_lookup(char *ip_addr, char *hostname, struct sockaddr_in *addr_con);
int reverse_dns_lookup(char *ip_addr, char *host);
int err(char *str);
unsigned int checksum(void *data, size_t len);
void calculate_rtt(struct stats *stats);
