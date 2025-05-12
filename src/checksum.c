#include <stddef.h>

unsigned short checksum(void *data, int len)
{
	unsigned short *cpy = data;
	unsigned int sum = 0;

	for (int i = 0; i < len; i += 2)
		sum += *cpy++;
	sum = (sum >> 16) + (sum & 0xFFFF);
	sum += (sum >> 16);
	return ~sum;
}
