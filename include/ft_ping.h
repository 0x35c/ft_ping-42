#pragma once

#include <netdb.h>
#include <netinet/ip_icmp.h>

#define PACKET_SIZE 64

struct ping_packet {
	struct icmphdr hdr;
	char payload[PACKET_SIZE - sizeof(struct icmphdr)];
};

struct rtt {
	double min;
	double avg;
	double max;
};

struct stats {
	int packetsize;
	char host[NI_MAXHOST];
	char ip[15];
	int req_count;
	int ttl;
	struct rtt rtt;
};

unsigned int checksum(void *data, size_t len);
void ping(int sockfd, struct sockaddr_in *addr_con, struct stats *stats,
          char *host_input);
int dns_lookup(char *ip_addr, char *hostname, struct sockaddr_in *addr_con);
int reverse_dns_lookup(char *ip_addr, char *host);
int err(char *str);
void calculate_rtt(struct stats *stats);
