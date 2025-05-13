#include <ctype.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ft_ping.h"

int get_option_arg(struct option *options, e_flag flag)
{
	if (!options)
		return 0;
	for (struct option *it = options; it; it = it->next)
		if (it->flag == flag)
			return it->arg;
	return 0;
}

static int add_option(struct option **head, e_flag flag, int arg)
{
	struct option *new_option = malloc(sizeof(struct option));
	if (!new_option) {
		dprintf(2, "ft_ping: allocation of option failed\n");
		for (struct option *it = *head; it;) {
			struct option *tmp = it;
			it = it->next;
			free(tmp);
		}
		return -1;
	}

	new_option->flag = flag;
	new_option->arg = arg;
	new_option->next = NULL;

	if (*head == NULL) {
		*head = new_option;
		return 0;
	}
	for (struct option *it = *head; it; it = it->next) {
		if (it->next == NULL) {
			it->next = new_option;
			break;
		}
	}
	return 0;
}

static int find_arg(e_flag flag, char *s)
{
	for (size_t i = 0; s[i]; i++) {
		if (!isdigit(s[i])) {
			dprintf(2, "ft_ping: invalid argument: '%s'\n", s);
			return -1;
		}
	}
	long arg = atol(s);
	if (arg > INT_MAX) {
		dprintf(2,
		        "ft_ping: invalid argument: '%s': Numerical "
		        "result out of range\n",
		        s);
		return -1;
	}
	switch (flag) {
	case COUNT:
		return arg;
	case INTERVAL:
		return arg * 1000000;
	case SIZE:
		if (arg < 0 || arg > 65527)
			dprintf(
			    2,
			    "ft_ping: invalid -s value: '%s': out of range: 0 "
			    "<= value <= 65527\n",
			    s);
		else
			return arg;
	case TTL:
		if (arg == 0)
			dprintf(2, "ft_ping: cannot set unicast time-to-live: "
			           "Invalid argument\n");
		else if (arg < 0 || arg > 255)
			dprintf(
			    2,
			    "ft_ping: invalid argument: '%s': out of range: 0 "
			    "<= value <= 255\n",
			    s);
		else
			return arg;
	default:
		break;
	}
	return -1;
}

static int find_flag(char *s)
{
	// That means we're now done with options and are onto the hostname
	if (s[0] != '-')
		return -1;
	if (!strncmp(s, "--ttl", 5))
		return TTL;
	if (strlen(s) > 2) {
		dprintf(
		    2,
		    "ft_ping: invalid option '%s' (please put a single char)\n",
		    s);
		return 0;
	}
	switch (s[1]) {
	case 'c':
		return COUNT;
	case 'i':
		return INTERVAL;
	case 'q':
		return QUIET;
	case 's':
		return SIZE;
	case 'v':
		return VERBOSE;
	default:
		break;
	}
	return 0;
}

struct option *parse_options(char **options, char **hostname)
{
	struct option *head = NULL;
	int flag = 0;
	int arg = 0;

	for (size_t i = 0; options[i]; i++) {
		if (!flag) {
			flag = find_flag(options[i]);
			switch (flag) {
			case QUIET:
				if (add_option(&head, flag, -1))
					return NULL;
				flag = 0;
				continue;
			case VERBOSE:
				if (add_option(&head, flag, -1))
					return NULL;
				flag = 0;
				continue;
			case -1:
				*hostname = options[i];
				return head;
			case 0:
				return NULL;
			default:
				break;
			}
		}
		if (options[i + 1] == NULL) {
			dprintf(2,
			        "ft_ping: invalid option '%s' requires an "
			        "argument\n",
			        options[i]);
			return NULL;
		}
		i++;
		arg = find_arg(flag, options[i]);
		if (arg < 0)
			return NULL;
		if (add_option(&head, flag, arg) < 0)
			return NULL;
		flag = 0;
	}
	return head;
}
