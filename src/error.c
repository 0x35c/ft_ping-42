#include <errno.h>
#include <stdio.h>

int err(char *str)
{
	int err = errno;
	perror(str);
	return err;
}

void print_usage(void)
{
	dprintf(
	    2,
	    "Usage: ./ft_ping [OPTIONS] destination\n"
	    "Options:\n"
	    "  -t TTL         Set the IP time to live\n"
	    "  -c COUNT       Stop after sending COUNT ECHO_REQUEST packets\n"
	    "  -i INTERVAL    Wait INTERVAL seconds between sending each "
	    "packet\n"
	    "  -q             Quiet output. Nothing is displayed except the "
	    "summary\n"
	    "  -s SIZE        Use SIZE as the number of data bytes to be sent\n"
	    "  -v             Verbose output\n");
}
