#include <arpa/inet.h>
#include <assert.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

#include "ft_ping.h"

bool sending = true;

static void sigint_handler(int signum)
{
	(void)signum;
	sending = false;
}

static int send_packet(int sockfd, struct sockaddr_in *addr_con,
                       int payload_size, int packets_sent)
{
	struct icmphdr hdr;

	bzero(&hdr, sizeof(hdr));
	hdr.type = ICMP_ECHO;
	hdr.un.echo.id = getpid();
	hdr.un.echo.sequence = packets_sent;
	char *payload = malloc(payload_size);
	if (!payload) {
#ifdef DEBUG
		dprintf(2, "Failed to allocate the payload\n");
#endif
		return -1;
	}
	memset(payload, '*', payload_size);

	size_t packet_size = sizeof(hdr) + payload_size;
	char *packet = malloc(packet_size);
	if (!packet) {
#ifdef DEBUG
		dprintf(2, "Failed to allocate the packet\n");
#endif
		free(payload);
		return -1;
	}
	memcpy(packet, &hdr, sizeof(hdr));
	memcpy(packet + sizeof(hdr), payload, payload_size);
	struct icmphdr *hdr_packet = (struct icmphdr *)packet;
	hdr_packet->checksum = checksum(packet, packet_size);

	if (sendto(sockfd, packet, packet_size, 0, (struct sockaddr *)addr_con,
	           sizeof(*addr_con)) <= 0) {
#ifdef DEBUG
		dprintf(2, "Failed to send packet\n");
#endif
		free(payload);
		free(packet);
		return -1;
	};
	free(payload);
	free(packet);
	return 0;
}

static const char *icmp_type_desc(uint8_t type)
{
	switch (type) {
	case 0:
		return "Echo reply";
	case 3:
		return "Destination unreachable";
	case 5:
		return "Redirect message";
	case 8:
		return "Echo request";
	case 11:
		return "Time to live exceeded";
	case 12:
		return "Parameter problem";
	default:
		return "Unknown type";
	}
}

static int receive_packet(int sockfd, bool verbose, int packets_received)
{
	char data[128];
	struct sockaddr_in addr_recv;

	socklen_t len = sizeof(addr_recv);
	if (recvfrom(sockfd, data, sizeof(data), 0,
	             (struct sockaddr *)&addr_recv, &len) <= 0) {
#ifdef DEBUG
		dprintf(2, "Failed to receive packet\n");
#endif
		return -1;
	}

	struct icmphdr *recv_hdr =
	    (struct icmphdr *)(data + sizeof(struct iphdr));
	if (recv_hdr->type != 0 || recv_hdr->code != 0) {
		dprintf(2, "From %s icmp_seq=%d %s\n",
		        inet_ntoa(addr_recv.sin_addr), packets_received,
		        icmp_type_desc(recv_hdr->type));
		if (verbose)
			dprintf(2, "ICMP %s (%d) code %d from %s\n",
			        icmp_type_desc(recv_hdr->type), recv_hdr->type,
			        recv_hdr->code, inet_ntoa(addr_recv.sin_addr));
		return -1;
	}
	return 0;
}

void ping(int sockfd, struct sockaddr_in *addr_con, struct option_lst *options,
          struct stats *stats, char *host_input)
{
	struct timespec packet_start, packet_end, tfs, tfe;
	struct icmphdr recv_hdr;
	int packets_sent = 0, packets_received = 0;
	int interval, count, quiet;
	bool verbose = false;
	long double total_ms;

	bzero(&recv_hdr, sizeof(recv_hdr));
	signal(SIGINT, sigint_handler);

	count = get_option_arg(options, FL_COUNT);
	interval = get_option_arg(options, FL_INTERVAL);
	if (!interval)
		interval = 1000000;
	quiet = get_option_arg(options, FL_QUIET);

	printf("PING %s (%s) %lu(%lu) bytes of data.\n", host_input, stats->ip,
	       stats->packetsize,
	       stats->packetsize + sizeof(struct icmphdr) +
	           sizeof(struct iphdr));
	if (get_option_arg(options, FL_VERBOSE))
		verbose = true;

	clock_gettime(CLOCK_MONOTONIC, &tfs);
	while (1) {
		usleep(interval);
		if (!sending)
			break;

		clock_gettime(CLOCK_MONOTONIC, &packet_start);
		if (send_packet(sockfd, addr_con, stats->packetsize,
		                packets_sent))
			continue;
		packets_sent++;

		if (receive_packet(sockfd, verbose, packets_sent)) {
			if (count && packets_sent >= count)
				sending = false;
			continue;
		}
		if (!quiet && (recv_hdr.type != 0 || recv_hdr.code != 0)) {
			printf("%lu bytes from %s (%s): icmp_seq=%d ttl=%d "
			       "%s ms\n",
			       stats->packetsize + sizeof(struct icmphdr),
			       stats->host, stats->ip, packets_sent, stats->ttl,
			       icmp_type_desc(recv_hdr.type));
			continue;
		}
		packets_received++;
		clock_gettime(CLOCK_MONOTONIC, &packet_end);

		long double time_elapsed =
		    (packet_end.tv_sec - packet_start.tv_sec) * 1000.0 +
		    (packet_end.tv_nsec - packet_start.tv_nsec) / 1e6;
		assert(time_elapsed >= 0);
		if (stats->rtt.min > time_elapsed || stats->rtt.min == 0)
			stats->rtt.min = time_elapsed;
		if (stats->rtt.max < time_elapsed)
			stats->rtt.max = time_elapsed;
		stats->rtt.avg += time_elapsed;

		if (!quiet)
			printf("%lu bytes from %s (%s): icmp_seq=%d ttl=%d "
			       "time=%.1Lf ms\n",
			       stats->packetsize + sizeof(struct icmphdr),
			       stats->host, stats->ip, packets_sent, stats->ttl,
			       time_elapsed);
	}
	clock_gettime(CLOCK_MONOTONIC, &tfe);

	stats->rtt.avg /= packets_sent;
	long double time_elapsed =
	    ((double)(tfe.tv_nsec - tfs.tv_nsec)) / 1000000.0;
	total_ms = (tfe.tv_sec - tfs.tv_sec) * 1000.0 + time_elapsed;
	double loss_ratio =
	    ((double)(packets_sent - packets_received) / packets_sent) * 100.0;
	printf("\n--- %s ping statistics ---\n", host_input);
	printf("%d packets transmitted, %d received, %.1lf%% packet loss, time "
	       "%.1Lfms\n",
	       packets_sent, packets_received, loss_ratio, total_ms);
	if (loss_ratio < 100.0)
		printf("rtt min/avg/max = %.3lf/%.3lf/%.3lf ms\n",
		       stats->rtt.min, stats->rtt.avg, stats->rtt.max);
}
