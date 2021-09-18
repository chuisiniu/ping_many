#include "ping.h"

#include <stdlib.h>

extern target_set_t *echo_create_target_set(char **ip_array, int ip_array_len, int pkt_num);
void echo(target_set_t *targets);


int main()
{
	char *ip_array[] = {"192.168.24.35", "192.168.24.152", "192.168.24.71"};
	target_set_t *targets;

	targets = echo_create_target_set(ip_array, 3, 4);

	echo(targets);

	exit(0);
}
