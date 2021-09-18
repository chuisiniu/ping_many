#include <gtest/gtest.h>

extern "C" {
#include <linux/types.h>

extern int addr_get_subnet_range(const char *str, __be32 *begin, __be32 *end);
}

TEST(AddrTest, Subnet) {
	__be32 begin;
	__be32 end;

	EXPECT_EQ(0, addr_get_subnet_range("192.168.24.0/24", &begin, &end));
}
