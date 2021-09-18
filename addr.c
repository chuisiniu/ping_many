#include "ping.h"
#include <arpa/inet.h>

int addr_get_subnet_range(const char *str, __be32 *begin, __be32 *end)
{
	char   ip[16];
	char   mask[3];
	char  *tmp;
	char  *tmp_end;
	int    prefix_len;
	__be32 m;

	int i;

	bzero(ip, sizeof(ip));
	bzero(mask, sizeof(mask));

	tmp     = ip;
	tmp_end = ip + sizeof(ip);
	for (i = 0; str[i]; i++) {
		if ('/' == str[i]) {
			tmp     = mask;
			tmp_end = mask + sizeof(mask);

			continue;
		}

		if ((str[i] < '0' || str[i] > '9') && str[i] != '.')
			return -1;

		if (tmp >= tmp_end)
			return -1;

		*(tmp++) = str[i];
	}

	if (-1 == inet_pton(AF_INET, ip, begin))
		return -1;

	prefix_len = atoi(mask);
	if (prefix_len <= 0 || prefix_len >= 32)
		return -1;

	m = htonl(~((1 << (32 - prefix_len)) - 1));
	*begin = *begin & m;
	*end   = (~m) | (*begin);

	return 0;
}
