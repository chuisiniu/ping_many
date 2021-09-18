#ifndef _PING_H_
#define _PING_H_

#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <asm/byteorder.h>
#include <stdint.h>
#include <stdio.h>
#include <netinet/in.h>
#include <stddef.h>
#include <linux/types.h>
#include <fcntl.h>

typedef struct ip_header {
#if defined(__LITTLE_ENDIAN_BITFIELD)
	__u8	header_length:4,
		version:4;
#elif defined (__BIG_ENDIAN_BITFIELD)
	__u8	version:4,
		header_length:4;
#else
#error	"Please fix <asm/byteorder.h>"
#endif
	__u8	type_of_service;
	__be16	total_length;
	__be16	identification;
#if defined(__LITTLE_ENDIAN_BITFIELD)
	__be16	flags:3,
		fragment_offset:13;
#elif defined (__BIG_ENDIAN_BITFIELD)
	__u8	fragment_offset:13,
		flags:3;
#else
#error	"Please fix <asm/byteorder.h>"
#endif
	__u8	time_to_live;
	__u8	protocol;
	__sum16	checksum;
	__be32	saddr;
	__be32	daddr;
	/*The options start here. */
} ip_header_t;

typedef struct icmp_header {
	uint8_t type;
	uint8_t code;
	__be16  checksum;
} __attribute__((__packed__)) icmp_header_t;

typedef struct icmp_echo_msg {
	icmp_header_t header;
	__be16        identifier;
	__be16        sequence;
	uint8_t       data[0];
} __attribute__((__packed__)) icmp_echo_msg_t;

typedef struct target {
	struct sockaddr_in ip;
	int                remain_pkt;
	__be16             id;
	__be16             seq;
	time_t             req_time;
	int                waiting_rsp;
} target_t;

typedef struct target_set {
	int                 num;
	int                 remain_pkt;
	int                 waiting_target_num;
	unsigned char      *content;
	int                 content_len;
	struct sockaddr_in  src;
	target_t            targets[0];
} target_set_t;

#endif /* _PING_H_ */
