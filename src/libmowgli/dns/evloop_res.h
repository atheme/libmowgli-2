/*
 * res.h for referencing functions in res.c, reslib.c
 *
 * Originally from Charybdis (before that, hybrid), but very little of the
 * original remains, so...
 */

#ifndef __MOWGLI_DNS_RES_H__
#define __MOWGLI_DNS_RES_H__

/* Maximum number of nameservers we track */
#define MOWGLI_DNS_MAXNS 10

typedef struct
{
	mowgli_vio_sockaddr_t nsaddr_list[MOWGLI_DNS_MAXNS];
	int nscount;

	int retrycnt;

	int timeout_count[MOWGLI_DNS_MAXNS];

	mowgli_vio_t *vio;
	mowgli_eventloop_t *eventloop;
	mowgli_eventloop_timer_t *timeout_resolver_timer;

	mowgli_list_t request_list;

	mowgli_random_t *rand;

	const char *resolvconf;
	bool dns_init;

	char domain[MOWGLI_DNS_RES_HOSTLEN];
} mowgli_dns_evloop_t;

extern int mowgli_dns_evloop_init(mowgli_dns_t *dns, mowgli_eventloop_t *eventloop);
extern int mowgli_dns_evloop_restart(mowgli_dns_t *dns);
extern void mowgli_dns_evloop_destroy(mowgli_dns_t *dns);
extern void mowgli_dns_evloop_delete_queries(mowgli_dns_t *dns, const mowgli_dns_query_t *);
extern void mowgli_dns_evloop_gethost_byname(mowgli_dns_t *dns, const char *, mowgli_dns_query_t *, int);
extern void mowgli_dns_evloop_gethost_byaddr(mowgli_dns_t *dns, const struct sockaddr_storage *, mowgli_dns_query_t *);

extern void mowgli_dns_evloop_add_local_domain(mowgli_dns_t * dns, char *, size_t);
extern int mowgli_dns_evloop_set_resolvconf(mowgli_dns_t *dns, const char *respath);

extern const mowgli_dns_ops_t mowgli_dns_evloop_resolver;

#endif
