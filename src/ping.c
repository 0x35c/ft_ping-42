#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

#include "ft_ping.h"

#define PING_SLEEP_RATE 1000000

bool sending = true;

static void sigint_handler(int signum)
{
	(void)signum;
	sending = false;
}

static int receive_packet(int sockfd)
{
	char data[128];
	struct sockaddr_in addr_recv;

	socklen_t len = sizeof(addr_recv);
	if (recvfrom(sockfd, data, sizeof(data), 0,
	             (struct sockaddr *)&addr_recv, &len) <= 0) {
		dprintf(2, "Failed to receive packet\n");
		return -1;
	}

	struct icmphdr *recv_hdr =
	    (struct icmphdr *)(data + sizeof(struct iphdr));
	if (recv_hdr->type != 0 || recv_hdr->code != 0) {
		dprintf(2,
		        "Received packet with ICMP type %d code "
		        "%d\n",
		        recv_hdr->type, recv_hdr->code);
		return -1;
	}
	return 0;
}

static int send_packet(int sockfd, struct sockaddr_in *addr_con,
                       int packets_sent)
{
	struct ping_packet packet;

	bzero(&packet, sizeof(packet));
	packet.hdr.type = ICMP_ECHO;
	packet.hdr.un.echo.id = getpid();
	packet.hdr.un.echo.sequence = packets_sent;
	memset(packet.payload, '*', sizeof(packet.payload));
	packet.hdr.checksum = checksum(&packet, sizeof(packet));

	if (sendto(sockfd, &packet, sizeof(packet), 0,
	           (struct sockaddr *)addr_con, sizeof(*addr_con)) <= 0) {
		dprintf(2, "Failed to send packet\n");
		return -1;
	};
	return 0;
}

void ping(int sockfd, struct sockaddr_in *addr_con, struct stats *stats,
          char *host_input)
{
	struct timespec packet_start, packet_end, tfs, tfe;
	int packets_sent = 0, packets_received = 0;
	long double total_ms;

	signal(SIGINT, sigint_handler);

	// Get the time before sending all the packets
	clock_gettime(CLOCK_MONOTONIC, &tfs);

	while (1) {
		usleep(PING_SLEEP_RATE);
		if (!sending)
			break;

		clock_gettime(CLOCK_MONOTONIC, &packet_start);

		if (send_packet(sockfd, addr_con, packets_sent))
			continue;
		packets_sent++;

		if (receive_packet(sockfd))
			continue;
		packets_received++;

		clock_gettime(CLOCK_MONOTONIC, &packet_end);

		long double time_elapsed =
		    (packet_end.tv_sec - packet_start.tv_sec) * 1000.0 +
		    (packet_end.tv_nsec - packet_start.tv_nsec) / 1e6;
		if (time_elapsed < 0)
			printf("end: %ld - start: %ld\n", packet_end.tv_nsec,
			       packet_start.tv_nsec);
		if (stats->rtt.min > time_elapsed || stats->rtt.min == 0)
			stats->rtt.min = time_elapsed;
		if (stats->rtt.max < time_elapsed)
			stats->rtt.max = time_elapsed;
		stats->rtt.avg += time_elapsed;

		printf(
		    "%d bytes from %s (%s): icmp_seq=%d ttl=%d time=%.1Lf ms\n",
		    stats->packetsize, stats->host, stats->ip, packets_sent,
		    stats->ttl, time_elapsed);
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
	printf("rtt min/avg/max = %.3lf/%.3lf/%.3lf ms\n", stats->rtt.min,
	       stats->rtt.avg, stats->rtt.max);
}
