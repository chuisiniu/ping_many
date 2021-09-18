#include <gtest/gtest.h>

extern "C" {
#include <linux/types.h>

	extern int addr_get_subnet_range(const char *str, __be32 *begin, __be32 *end);
}

void check_ip(__be32 ip, uint8_t a0, uint8_t a1, uint8_t a2, uint8_t a3)
{
	uint8_t *arr;

	arr = (uint8_t *)&ip;
	EXPECT_EQ(arr[0], a0);
	EXPECT_EQ(arr[1], a1);
	EXPECT_EQ(arr[2], a2);
	EXPECT_EQ(arr[3], a3);
}

TEST(AddrTest, Subnet) {
	__be32   begin;
	__be32   end;

	EXPECT_EQ(0, addr_get_subnet_range("192.168.24.0/24", &begin, &end));
	check_ip(begin, 192, 168, 24, 0);
	check_ip(end,   192, 168, 24, 255);

	EXPECT_EQ(-1, addr_get_subnet_range("192.168.24.0//24", &begin, &end));
	EXPECT_EQ(-1, addr_get_subnet_range("192.168.24.0/24a", &begin, &end));
	EXPECT_EQ(-1, addr_get_subnet_range("192.168.24.0/2a4", &begin, &end));
	EXPECT_EQ(-1, addr_get_subnet_range("192.168.24.a0/2a4", &begin, &end));
	EXPECT_EQ(-1, addr_get_subnet_range("192.168.240.2401/2a4", &begin, &end));
	EXPECT_EQ(-1, addr_get_subnet_range("192.168.24.0/0", &begin, &end));
	EXPECT_EQ(-1, addr_get_subnet_range("192.168.24.0/32", &begin, &end));
}
