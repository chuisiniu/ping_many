#include "ping.h"

#define ICMP_ECHO_REQUEST_TYPE 8
#define ICMP_ECHO_REQUEST_CODE 0

int net_create_raw_socket(int buf_size, int allow_broadcast)
{
	int fd;
	int ip_hdrincl;
	int flag;

	fd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if (fd < 0) {
		perror("create raw socket");

		return -1;
	}

	flag = fcntl(fd, F_GETFL, 0);
	if (-1 == flag) {
		perror("fcntl get");

		goto ERROR;
	}
	flag |= O_NONBLOCK;
	flag = fcntl(fd, F_SETFL, flag);
	if (-1 == flag) {
		perror("fcntl set");

		goto ERROR;
	}

	if (0 < setsockopt(fd, SOL_SOCKET, SO_RCVBUF, &buf_size, sizeof(buf_size))) {
		perror("set socket rcvbuf");

		goto ERROR;
	}

	if (0 < setsockopt(fd, SOL_SOCKET, SO_BROADCAST, &allow_broadcast,
					   sizeof(allow_broadcast))) {
		perror("set socket rcvbuf");

		goto ERROR;
	}

	ip_hdrincl = 1;
	if (0 < setsockopt(fd, IPPROTO_IP, IP_HDRINCL, &ip_hdrincl,
					   sizeof(ip_hdrincl))) {
		perror("set ip hdrincl");

		goto ERROR;
	}

	return fd;
ERROR:
	close(fd);

	return -1;
}

int net_send_ip(int fd, struct sockaddr_in *dst, unsigned char *content, int len)
{
	char buf[1408];

	len = len < (int)sizeof(buf) ? len : (int)sizeof(buf);
	memcpy(buf, content, len);

	if (sendto(fd, buf, len, 0, (struct sockaddr *)dst, sizeof(*dst)) < 0) {
		perror("sendto");

		return -1;
	}

	return 0;

}

__sum16 net_calc_checksum(uint8_t *buf, int len)
{
	long      sum;
	uint16_t *addr;

	sum  = 0;
	addr = (uint16_t *)buf;

	while(len > 1) {
		sum += *addr++;
		len -= sizeof(*addr);
	}

	if (len > 0)
		sum += *((uint8_t *)addr);

	while (sum >> 16)
		sum = (sum & 0xFFFF) + (sum >> 16);

	return (unsigned short)(~sum);
}

__be16 net_get_ip_identification()
{
	static __be16 identification = 0;

	return ntohs(identification++);
}

void net_make_ip_pkt(
	ip_header_t        *ip_header,
	struct sockaddr_in *src,
	struct sockaddr_in *dst,
	int                 payload_len)
{
	bzero(ip_header, sizeof(*ip_header));

	ip_header->header_length   = sizeof(*ip_header) / 4;
	ip_header->version         = 4;
	ip_header->type_of_service = 0;
	ip_header->total_length    = htons(sizeof(*ip_header) + payload_len);
	ip_header->identification  = htons(net_get_ip_identification());
	ip_header->flags           = 0;
	ip_header->fragment_offset = 0;
	ip_header->protocol        = IPPROTO_ICMP;
	ip_header->time_to_live    = 255;
	ip_header->saddr           = src->sin_addr.s_addr;
	ip_header->daddr           = dst->sin_addr.s_addr;
	ip_header->checksum        = net_calc_checksum((uint8_t *)ip_header,
												   sizeof(*ip_header));
}

void net_make_icmp_echo_msg(
	unsigned char *msg_buf,
	int            msg_buf_len,
	__be16         id,
	__be16         seq,
	unsigned char *content,
	int            content_len)
{
	icmp_echo_msg_t *icmp;

	bzero(msg_buf, msg_buf_len);
	icmp                  = (icmp_echo_msg_t *)msg_buf;
	icmp->header.type     = ICMP_ECHO_REQUEST_TYPE;
	icmp->header.code     = ICMP_ECHO_REQUEST_CODE;
	icmp->identifier      = id;
	icmp->sequence        = seq;
	if (content_len > msg_buf_len - (int)offsetof(icmp_echo_msg_t, data))
		content_len = msg_buf_len - (int)offsetof(icmp_echo_msg_t, data);
	memcpy(icmp->data, content, content_len);

	icmp->header.checksum = net_calc_checksum((uint8_t *)icmp,
											  sizeof(*icmp) + content_len);
}

int net_send_icmp(
	int                 fd,
	struct sockaddr_in *src,
	struct sockaddr_in *dst,
	__be16              id,
	__be16              seq,
	unsigned char      *content,
	int                 content_len)
{
	unsigned char    buf[1408];
	ip_header_t     *ip_h;
	icmp_echo_msg_t *icmp;
	int              pkt_len;

	ip_h = (ip_header_t *)buf;
	icmp = (icmp_echo_msg_t *)(buf + sizeof(*ip_h));
	pkt_len = sizeof(*ip_h) + sizeof(icmp_echo_msg_t) + content_len;

	net_make_icmp_echo_msg((unsigned char *)icmp, sizeof(buf) - sizeof(*ip_h),
						   id, seq, content, content_len);

	net_make_ip_pkt(ip_h, src, dst, pkt_len);

	return net_send_ip(fd, dst, buf, pkt_len);
}

int net_rcv(int fd, unsigned char *rcv_buf, int rcv_buf_len, int *data_len)
{
	unsigned char      buf[1408];
	int                rcv_len;

	rcv_len = recv(fd, buf, sizeof(buf), 0);
	if (rcv_len < (int)(sizeof(ip_header_t) + sizeof(icmp_echo_msg_t)))
		return 0;

	*data_len = rcv_len < rcv_buf_len ? rcv_len : rcv_buf_len;
	memcpy(rcv_buf, buf, *data_len);

	return 1;
}
