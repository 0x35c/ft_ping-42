#pragma once

#include <netdb.h>
#include <netinet/ip_icmp.h>

#define PACKET_SIZE 64

struct ping_packet {
	struct icmphdr hdr;
	char payload[PACKET_SIZE - sizeof(struct icmphdr)];
};

struct stats {
	int packetsize;
	char *domain_name;
	char *domain_ip;
	int req_count;
	int ttl;
	double rtt;
};

unsigned int checksum(void *data, size_t len);
void ping(int sockfd, struct sockaddr_in *addr_con, struct stats *stats);
int dns_lookup(char *ip_addr, char *hostname, struct sockaddr_in *addr_con);
int err(char *str);
