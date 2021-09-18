#include "ping.h"

#include <sys/select.h>
#include <arpa/inet.h>
#include <time.h>

#define ICMP_ECHO_DEFAULT_REQUEST_CONTENT "hello world"

extern int net_create_raw_socket(int buf_size, int allow_broadcast);
extern int net_send_icmp(
	int                 fd,
	struct sockaddr_in *src,
	struct sockaddr_in *dst,
	__be16              id,
	__be16              seq,
	unsigned char      *content,
	int                 content_len);
extern int net_rcv(int fd, unsigned char *rcv_buf, int rcv_buf_len, int *data_len);

target_set_t *echo_create_target_set(char **ip_array, int ip_array_len, int pkt_num)
{
	target_set_t *set;
	target_t     *target;
	int           i;

	set              = malloc(sizeof(*set) + sizeof(target_t) * ip_array_len);
	set->num         = 0;
	set->remain_pkt  = 0;
	set->content     = NULL;
	set->content_len = 0;

	if (0 == inet_pton(AF_INET, "192.168.24.250", &set->src.sin_addr.s_addr)) {
		perror("inet_pton");

		free(set);

		return NULL;
	}
	set->src.sin_family = AF_INET;
	set->content        = ICMP_ECHO_DEFAULT_REQUEST_CONTENT;
	set->content_len    = sizeof(ICMP_ECHO_DEFAULT_REQUEST_CONTENT) - 1;

	for (i = 0; i < ip_array_len; i++) {
		target = &set->targets[set->num];
		if (!inet_pton(AF_INET, ip_array[i], &target->ip.sin_addr.s_addr)) {
			perror("inet_pton");

			continue;
		}
		target->ip.sin_family = AF_INET;

		target->remain_pkt  = pkt_num;
		target->id          = htons((short)set->num);
		target->seq         = htons(0);
		target->waiting_rsp = 0;
		target->req_time    = 0;

		set->num++;
		set->remain_pkt += target->remain_pkt;
	}

	return set;
}

int echo_request(
	int                 fd,
	struct sockaddr_in *src,
	target_t           *target,
	unsigned char      *content,
	int                 content_len)
{
	if (target->remain_pkt) {
		net_send_icmp(fd, src, &target->ip, target->id, target->seq++,
					  content, content_len);
		target->req_time    = time(NULL);
		target->waiting_rsp = 1;
		target->remain_pkt--;

		return 1;
	}

	return 0;
}

void echo_rcv_response(int fd, target_set_t *targets)
{
	unsigned char    buf[1408];
	int              rcv_len;
	ip_header_t     *ip_h;
	icmp_echo_msg_t *rsp;
	target_t        *target;

	if (0 == net_rcv(fd, buf, sizeof(buf), &rcv_len))
		return;

	ip_h = (ip_header_t *)buf;
	rsp  = (icmp_echo_msg_t *)(buf + sizeof(*ip_h));

	if(ntohs(rsp->identifier) > targets->num) {
		fprintf(stderr, "invalid identifier");

		return;
	}
	target = &targets->targets[ntohs(rsp->identifier)];
	target->waiting_rsp = 0;

	printf("From %d.%d.%d.%d icmp_seq=%d\n",
		   ((unsigned char *)(&ip_h->saddr))[0],
		   ((unsigned char *)(&ip_h->saddr))[1],
		   ((unsigned char *)(&ip_h->saddr))[2],
		   ((unsigned char *)(&ip_h->saddr))[3],
		   rsp->sequence);
}

void echo(target_set_t *targets)
{
	int             fd;
	fd_set          rfds;
	fd_set          wfds;
	struct timeval  tv;
	int             retval;
	int             target_cur;
	target_t       *target;

	fd = net_create_raw_socket(1024 * targets->num, 1);
	if (fd < 0)
		return;

	target_cur = 0;
	while(targets->remain_pkt) {
		FD_ZERO(&rfds);
		FD_SET(fd, &rfds);
		FD_ZERO(&wfds);
		FD_SET(fd, &wfds);

		tv.tv_sec  = 1;
		tv.tv_usec = 0;

		target = &targets->targets[target_cur];

		retval = select(fd + 1, &rfds, &wfds, NULL, &tv);
		if (-1 == retval) {
			perror("select");
		} else if (retval) {
			if (FD_ISSET(fd, &rfds)) {
				echo_rcv_response(fd, targets);
			}
			if (FD_ISSET(fd, &wfds)) {
				if (!target->waiting_rsp
					&& echo_request(fd, &targets->src, target,
								 targets->content, targets->content_len)) {
					targets->remain_pkt--;
				}
			}
		} else {
			return;
		}

		if (++target_cur == targets->num)
			target_cur = 0;
	}
}
