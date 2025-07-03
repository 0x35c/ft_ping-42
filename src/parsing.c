#include <ctype.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "ft_ping.h"

int get_option_arg(struct option_lst *options, e_flag flag)
{
	if (!options)
		return 0;
	for (struct option_lst *it = options; it; it = it->next)
		if (it->flag == flag)
			return it->arg;
	return 0;
}

void free_options(struct option_lst **head)
{
	for (struct option_lst *it = *head; it;) {
		struct option_lst *tmp = it;
		it = it->next;
		free(tmp);
	}
	*head = NULL;
}

static int add_option(struct option_lst **head, e_flag flag, int arg)
{
	struct option_lst *new_option = malloc(sizeof(struct option_lst));
	if (!new_option) {
		dprintf(2, "ft_ping: allocation of option failed\n");
		free_options(head);
		return -1;
	}

	new_option->flag = flag;
	new_option->arg = arg;
	new_option->next = NULL;

	if (*head == NULL) {
		*head = new_option;
		return 0;
	}
	for (struct option_lst *it = *head; it; it = it->next) {
		if (it->next == NULL) {
			it->next = new_option;
			break;
		}
	}
	return 0;
}

static int check_arg(int opt, const char *s)
{
	for (size_t i = 0; i < strlen(optarg); i++) {
		if (!isdigit(optarg[i])) {
			dprintf(2,
			        "ft_ping: argument '%s' to option -%c "
			        "is invalid (numeric argument required)\n",
			        optarg, opt);
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
	switch (opt) {
	case FL_SIZE:
		if (arg < 0 || arg > 65507) {
			dprintf(
			    2,
			    "ft_ping: invalid -s value: '%s': out of range: 0 "
			    "<= value <= 65507\n",
			    s);
			return -1;
		}
		break;
	case FL_TTL:
		if (arg == 0) {
			dprintf(2, "ft_ping: cannot set unicast time-to-live: "
			           "Invalid argument\n");
			return -1;
		} else if (arg < 0 || arg > 255) {
			dprintf(
			    2,
			    "ft_ping: invalid argument: '%s': out of range: 0 "
			    "<= value <= 255\n",
			    s);
			return -1;
		}
		break;
	default:
		break;
	}
	return 0;
}

int parse_options(int ac, char *const *av, char **hostname,
                  struct option_lst **head)
{
	int opt;

	while ((opt = getopt(ac, av, ":t:c:i:qs:v?")) != -1) {
		if (opt == '?') {
			if (strcmp(av[optind - 1], "-?") == 0) {
				print_usage();
				exit(0);
			} else {
				dprintf(2, "ft_ping: invalid option -- '%s'\n",
				        av[optind]);
				print_usage();
				return -1;
			}
		}
		if (strchr("qv", opt)) {
			add_option(head, opt, 1);
			continue;
		}
		if (!optarg) {
			dprintf(2,
			        "ft_ping: missing argument for option "
			        "-%c\n",
			        opt);
			return -1;
		}
		if (check_arg(opt, optarg) < 0)
			return -1;
		if (add_option(head, opt, atoi(optarg)) < 0)
			return -1;
	}
	if (optind < ac)
		*hostname = av[optind];
	return 0;
}
