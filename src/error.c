#include <errno.h>
#include <stdio.h>

int err(char *str)
{
	int err = errno;
	perror(str);
	return err;
}
