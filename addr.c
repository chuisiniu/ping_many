#include "ping.h"
#include <arpa/inet.h>

int addr_sep_subnet_or_range(
	const char *str,
	char        sep,
	char       *part1,
	int         part1_len,
	char       *part2,
	int         part2_len)
{
	char *tmp;
	char *tmp_end;
	int   i;

	bzero(part1, part1_len);
	bzero(part2, part2_len);

	tmp     = part1;
	tmp_end = part1 + part1_len;
	for (i = 0; str[i]; i++) {
		if (sep == str[i]) {
			if (tmp_end == part2 + part2_len)
				return -1;

			tmp     = part2;
			tmp_end = part2 + part2_len;

			continue;
		}

		if ((str[i] < '0' || str[i] > '9') && str[i] != '.')
			return -1;

		if (tmp >= tmp_end)
			return -1;

		*(tmp++) = str[i];
	}

	return 0;
}

int addr_get_subnet_range(const char *str, __be32 *begin, __be32 *end)
{
	char   ip[16];
	char   mask[3];
	int    prefix_len;
	__be32 m;

	if (-1 == addr_sep_subnet_or_range(str, '/', ip, sizeof(ip),
									   mask, sizeof(mask)))
		return -1;

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

int addr_get_range_range(const char *str, __be32 *begin, __be32 *end)
{
	char   ip1[16];
	char   ip2[16];

	if (-1 == addr_sep_subnet_or_range(str, '-', ip1, sizeof(ip1),
									   ip2, sizeof(ip2)))
		return -1;

	if (-1 == inet_pton(AF_INET, ip1, begin)
		|| -1 == inet_pton(AF_INET, ip2, end))
		return -1;

	if (ntohl(*begin) > ntohl(*end))
		return -1;

	return 0;
}
