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

void sigint_handler(int signum)
{
	(void)signum;
	sending = false;
}

void ping(int sockfd, struct sockaddr_in *addr_con, struct stats *stats)
{
	struct ping_packet packet;
	struct timespec packet_start, packet_end, tfs, tfe;
	struct sockaddr_in addr_recv;
	char data[128];
	int packets_sent = 0, packets_received = 0;
	long double total_ms;

	signal(SIGINT, sigint_handler);

	// Get the time before sending all the packets
	clock_gettime(CLOCK_MONOTONIC, &tfs);

	while (1) {
		bzero(&packet, sizeof(packet));
		packet.hdr.type = ICMP_ECHO;
		packet.hdr.un.echo.id = getpid();
		packet.hdr.un.echo.sequence = packets_sent;
		memset(packet.payload, '*', sizeof(packet.payload));
		packet.hdr.checksum = checksum(&packet, sizeof(packet));

		clock_gettime(CLOCK_MONOTONIC, &packet_start);

		usleep(PING_SLEEP_RATE);
		if (!sending)
			break;

		// Send packet
		if (sendto(sockfd, &packet, sizeof(packet), 0,
		           (struct sockaddr *)addr_con,
		           sizeof(*addr_con)) <= 0) {
			dprintf(2, "Failed to send packet\n");
			continue;
		}
		packets_sent++;

		// Receive packet
		socklen_t len = sizeof(addr_recv);
		if (recvfrom(sockfd, data, sizeof(data), 0,
		             (struct sockaddr *)&addr_recv, &len) <= 0) {
			dprintf(2, "Failed to receive packet\n");
			continue;
		}
		packets_received++;

		clock_gettime(CLOCK_MONOTONIC, &packet_end);

		struct icmphdr *recv_hdr =
		    (struct icmphdr *)(data + sizeof(struct iphdr));
		if (recv_hdr->type != 0 || recv_hdr->code != 0) {
			dprintf(2,
			        "Received packet with ICMP type %d code "
			        "%d\n",
			        recv_hdr->type, recv_hdr->code);
			continue;
		}

		long double time_elapsed =
		    ((double)(packet_end.tv_nsec - packet_start.tv_nsec)) /
		    1000000.0;
		printf(
		    "%d bytes from %s (%s): icmp_seq=%d ttl=%d time=%.1Lf ms\n",
		    stats->packetsize, stats->domain_name, stats->domain_ip,
		    packets_sent, stats->ttl, time_elapsed);
	}
	clock_gettime(CLOCK_MONOTONIC, &tfe);

	long double time_elapsed =
	    ((double)(tfe.tv_nsec - tfs.tv_nsec)) / 1000000.0;
	total_ms = (tfe.tv_sec - tfs.tv_sec) * 1000.0 + time_elapsed;
	double loss_ratio =
	    ((double)(packets_sent - packets_received) / packets_sent) * 100.0;
	printf("\n--- %s ping statistics ---\n", stats->domain_name);
	printf(
	    "%d packets sent, %d packets received, %.1lf%% packet loss, time "
	    "%.1Lfms\n",
	    packets_sent, packets_received, loss_ratio, total_ms);
}
