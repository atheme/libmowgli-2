/*
 * A rewrite of Darren Reeds original res.c As there is nothing
 * left of Darrens original code, this is now licensed by the hybrid group.
 * (Well, some of the function names are the same, and bits of the structs..)
 * You can use it where it is useful, free even. Buy us a beer and stuff.
 *
 * The authors takes no responsibility for any damage or loss
 * of property which results from the use of this software.
 *
 * July 1999 - Rewrote a bunch of stuff here. Change hostent builder code,
 *     added callbacks and reference counting of returned hostents.
 *     --Bleep (Thomas Helvey <tomh@inxpress.net>)
 *
 * This was all needlessly complicated for irc. Simplified. No more hostent
 * All we really care about is the IP -> hostname mappings. Thats all.
 *
 * Apr 28, 2003 --cryogen and Dianora
 *
 * DNS server flooding lessened, AAAA-or-A lookup removed, ip6.int support
 * removed, various robustness fixes
 *
 * 2006 --jilles and nenolod
 *
 * Clean up various crap, remove global state. Reindent because two space indent
 * is hideous. Also remove ancient assumptions that don't make sense anymore (e.g.,
 * libmowgli targets C99, which specifies an 8-bit char). Pack all this stuff into
 * its own namespace. Also gutted a lot of needless/one-use/few-line functions.
 * Jesus, what were they thinking...
 *
 * 2012 --Elizacat
 */

#include "mowgli.h"
#include "dns.h"

#define MOWGLI_DNS_MAXPACKET 1024	/* rfc sez 512 but we expand names so ... */
#define MOWGLI_DNS_RES_MAXALIASES 35	/* maximum aliases allowed */
#define MOWGLI_DNS_RES_MAXADDRS 35	/* maximum addresses allowed */
#define MOWGLI_DNS_AR_TTL 600	/* TTL in seconds for dns cache entries */

/* RFC 1104/1105 wasn't very helpful about what these fields should be named, so for now, we'll
   just name them this way. we probably should look at what named calls them or something. */
#define MOWGLI_DNS_TYPE_SIZE ((size_t) 2)
#define MOWGLI_DNS_CLASS_SIZE ((size_t) 2)
#define MOWGLI_DNS_TTL_SIZE ((size_t) 4)
#define MOWGLI_DNS_RDLENGTH_SIZE ((size_t) 2)
#define MOWGLI_DNS_ANSWER_FIXED_SIZE (MOWGLI_DNS_TYPE_SIZE + MOWGLI_DNS_CLASS_SIZE + MOWGLI_DNS_TTL_SIZE + MOWGLI_DNS_RDLENGTH_SIZE)

#define MOWGLI_DNS_MAXLINE 128

typedef struct
{
	mowgli_node_t node;
	int id;
	time_t ttl;
	char type;
	char queryname[MOWGLI_DNS_RES_HOSTLEN + 1];	/* name currently being queried */
	char retries;	/* retry counter */
	char sends;	/* number of sends (>1 means resent) */
	time_t sentat;
	time_t timeout;
	unsigned int lastns;	/* index of last server sent to */
	struct sockaddr_storage addr;

	char *name;
	mowgli_dns_query_t *query;	/* query callback for this request */
} mowgli_dns_reslist_t;

static mowgli_heap_t *reslist_heap = NULL;

#ifndef _WIN32
static int parse_resvconf(mowgli_dns_t *dns);

#else
static void parse_windows_resolvers(mowgli_dns_t *dns);

#endif

static void timeout_resolver(void *arg);
static void add_nameserver(mowgli_dns_t *dns, const char *arg);
static int res_ourserver(mowgli_dns_t *dns, const struct sockaddr_storage *inp);
static void rem_request(mowgli_dns_t *dns, mowgli_dns_reslist_t *request);
static mowgli_dns_reslist_t *make_request(mowgli_dns_t *dns, mowgli_dns_query_t *query);
static void do_query_name(mowgli_dns_t *dns, mowgli_dns_query_t *query, const char *name, mowgli_dns_reslist_t *request, int);
static void do_query_number(mowgli_dns_t *dns, mowgli_dns_query_t *query, const struct sockaddr_storage *, mowgli_dns_reslist_t *request);
static void query_name(mowgli_dns_t *dns, mowgli_dns_reslist_t *request);
static int send_res_msg(mowgli_dns_t *dns, const char *buf, int len, int count);
static void resend_query(mowgli_dns_t *dns, mowgli_dns_reslist_t *request);
static int check_question(mowgli_dns_reslist_t *request, mowgli_dns_resheader_t *header, char *buf, char *eob);
static int proc_answer(mowgli_dns_reslist_t *request, mowgli_dns_resheader_t *header, char *, char *);
static mowgli_dns_reslist_t *find_id(mowgli_dns_t *dns, int id);
static mowgli_dns_reply_t *make_dnsreply(mowgli_dns_reslist_t *request);
static void res_readreply(mowgli_eventloop_t *eventloop, mowgli_eventloop_io_t *io, mowgli_eventloop_io_dir_t dir, void *userdata);

/*
 * mowgli_dns_evloop_init - do everything we need to read the resolv.conf file
 * and initialize the resolver file descriptor if needed
 */
int
mowgli_dns_evloop_init(mowgli_dns_t *dns, mowgli_eventloop_t *eventloop)
{
	int i;
	mowgli_dns_evloop_t *state;

	if (dns->dns_state == NULL)
		dns->dns_state = mowgli_alloc(sizeof(mowgli_dns_evloop_t));

	dns->dns_type = MOWGLI_DNS_TYPE_ASYNC;

	if (!reslist_heap)
		reslist_heap = mowgli_heap_create(sizeof(mowgli_dns_reslist_t), 512, BH_LAZY);

	state = dns->dns_state;

	state->rand = mowgli_random_create();

	state->nscount = 0;

#ifndef _WIN32
	parse_resvconf(dns);
#else
	parse_windows_resolvers(dns);
#endif

	if (state->nscount == 0)
	{
		mowgli_log("couldn't get resolv.conf entries, falling back to localhost resolver");
		add_nameserver(dns, "127.0.0.1");
	}

	for (i = 0; i < state->nscount; i++)
		state->timeout_count[i] = 0;

	if (state->vio == NULL)
	{
		state->vio = mowgli_vio_create(dns);

		if (mowgli_vio_socket(state->vio, state->nsaddr_list[0].addr.ss_family, SOCK_DGRAM, 0) != 0)
		{
			mowgli_log("start_resolver(): unable to open UDP resolver socket: %s",
				   state->vio->error.string);
			return -1;
		}

		state->eventloop = eventloop;
		mowgli_vio_eventloop_attach(state->vio, state->eventloop, NULL);
		mowgli_pollable_setselect(state->eventloop, state->vio->io.e, MOWGLI_EVENTLOOP_IO_READ, res_readreply);
		state->timeout_resolver_timer = mowgli_timer_add(state->eventloop, "timeout_resolver", timeout_resolver, dns, 1);
	}

	return 0;
}

int
mowgli_dns_evloop_set_resolvconf(mowgli_dns_t *dns, const char *respath)
{
#ifndef _WIN32
	mowgli_dns_evloop_t *state = dns->dns_state;

	return_val_if_fail(dns, -1);

	state->resolvconf = respath;

	if (!state->dns_init)
		return mowgli_dns_evloop_restart(dns);

	return 0;
#else
	mowgli_log("Unimplemented on Windows. :(");
#endif
}

/*
 * mowgli_dns_evloop_restart - reread resolv.conf, reopen socket
 */
int
mowgli_dns_evloop_restart(mowgli_dns_t *dns)
{
	mowgli_dns_evloop_t *state = dns->dns_state;

	mowgli_dns_evloop_destroy(dns);
	return mowgli_dns_evloop_init(dns, state->eventloop);
}

/* mowgli_dns_evloop_destroy - finish us off */
void
mowgli_dns_evloop_destroy(mowgli_dns_t *dns)
{
	mowgli_dns_evloop_t *state = dns->dns_state;

	mowgli_vio_close(state->vio);
	mowgli_vio_destroy(state->vio);

	mowgli_timer_destroy(state->eventloop, state->timeout_resolver_timer);

	mowgli_free(state);
	dns->dns_state = NULL;
}

#ifndef _WIN32

/* parse_resvconf()
 * inputs - NONE
 * output - -1 if failure, 0 if success
 * side effects - fills in state->nsaddr_list
 */
static int
parse_resvconf(mowgli_dns_t *dns)
{
	char *p;
	char *opt;
	char *arg;
	const char *respath;
	char input[MOWGLI_DNS_MAXLINE];
	FILE *file;
	mowgli_dns_evloop_t *state = dns->dns_state;

	if (state->resolvconf)
		respath = state->resolvconf;
	else
		respath = "/etc/resolv.conf";

	if ((file = fopen(respath, "r")) == NULL)
	{
		mowgli_log("Failed to open %s: %s", respath, strerror(errno));
		return -1;
	}

	while (fgets(input, sizeof(input), file) != NULL)
	{
		/* blow away any newline */
		if ((p = strpbrk(input, "\r\n")) != NULL)
			*p = '\0';

		p = input;

		/* skip until something thats not a space is seen */
		while (isspace(*p))
		{
			p++;
		}

		/* if at this point, have a '\0' then continue */
		if (*p == '\0')
			continue;

		/* Ignore comment lines immediately */
		if ((*p == '#') || (*p == ';'))
			continue;

		/* skip until a space is found */
		opt = p;

		while (!isspace(*p) && *p != '\0')
		{
			p++;
		}

		if (*p == '\0')
			continue;			/* no arguments?.. ignore this line */

		/* blow away the space character */
		*p++ = '\0';

		/* skip these spaces that are before the argument */
		while (isspace(*p))
		{
			p++;
		}

		/* Now arg should be right where p is pointing */
		arg = p;

		if ((p = strpbrk(arg, " \t")) != NULL)
			*p = '\0';			/* take the first word */

		if (strcmp(opt, "domain") == 0)
			mowgli_strlcpy(state->domain, arg, sizeof(state->domain));
		else if (strcmp(opt, "nameserver") == 0)
			add_nameserver(dns, arg);
	}

	fclose(file);

	state->dns_init = true;

	return 0;
}

#else

extern int mowgli_dns_get_windows_nameservers(char *ret_buf, size_t ret_size);

static void
parse_windows_resolvers(mowgli_dns_t *dns)
{
	char ns_buf[4096];
	char *server;

	mowgli_dns_get_windows_nameservers(ns_buf, sizeof ns_buf);

	for (server = strtok(ns_buf, ","); server != NULL; server = strtok(NULL, ","))
		add_nameserver(dns, server);
}

#endif

/* add_nameserver()
 * input - either an IPV4 address in dotted quad or an IPV6 address in : format
 * output - NONE
 * side effects - entry in state->nsaddr_list is filled in as needed */
static void
add_nameserver(mowgli_dns_t *dns, const char *arg)
{
	struct addrinfo hints, *res;

	mowgli_dns_evloop_t *state = dns->dns_state;

#ifdef DEBUG
	mowgli_log("add_nameserver(): %s", arg);
#endif

	/* Done max number of nameservers? */
	if (state->nscount >= MOWGLI_DNS_MAXNS)
	{
		mowgli_log("Too many nameservers, ignoring %s", arg);
		return;
	}

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = PF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE | AI_NUMERICHOST;

	if (getaddrinfo(arg, "domain", &hints, &res))
		return;

	if (res == NULL)
		return;

	memcpy(&state->nsaddr_list[state->nscount].addr, res->ai_addr, res->ai_addrlen);
	state->nsaddr_list[state->nscount].addrlen = res->ai_addrlen;
	state->nscount++;
	freeaddrinfo(res);
}

/*
 * int
 * res_ourserver(dns, inp)
 *      looks up "inp" in state->nsaddr_list[]
 * returns:
 *      0  : not found
 *      >0 : found
 * author:
 *      paul vixie, 29may94
 *      revised for ircd, cryogen(stu) may03
 *      rewritten by Elizacat 25mar12
 */
static int
res_ourserver(mowgli_dns_t *dns, const struct sockaddr_storage *inp)
{
	int ns;
	mowgli_dns_evloop_t *state = dns->dns_state;

	for (ns = 0; ns < state->nscount; ns++)
	{
		const struct sockaddr_storage *srv = &state->nsaddr_list[ns].addr;

		/* could probably just memcmp(srv, inp, srv.ss_len) here but we'll air on the side of
		   caution - stu */
		switch (srv->ss_family)
		{
		case AF_INET6:
		{
			const struct sockaddr_in6 *v6 = (const struct sockaddr_in6 *) srv;
			const struct sockaddr_in6 *v6in = (const struct sockaddr_in6 *) inp;

			if ((srv->ss_family == inp->ss_family) && (v6->sin6_port == v6in->sin6_port))
				if ((memcmp(&v6->sin6_addr.s6_addr, &v6in->sin6_addr.s6_addr,
					    sizeof(struct in6_addr)) == 0) ||
				    (memcmp(&v6->sin6_addr.s6_addr, &in6addr_any,
					    sizeof(struct in6_addr)) == 0))
				{
					state->timeout_count[ns] = 0;
					return 1;
				}

			break;
		}
		case AF_INET:
		{
			const struct sockaddr_in *v4 = (const struct sockaddr_in *) srv;
			const struct sockaddr_in *v4in = (const struct sockaddr_in *) inp;

			if ((srv->ss_family == inp->ss_family) && (v4->sin_port == v4in->sin_port))
				if ((v4->sin_addr.s_addr == INADDR_ANY)
				    || (v4->sin_addr.s_addr == v4in->sin_addr.s_addr))
				{
					state->timeout_count[ns] = 0;
					return 1;
				}

			break;
		}
		default:
			break;
		}
	}

	return 0;
}

/*
 * timeout_query_list - Remove queries from the list which have been
 * there too long without being resolved.
 */
static time_t
timeout_query_list(mowgli_dns_t *dns, time_t now)
{
	mowgli_node_t *ptr;
	mowgli_node_t *next_ptr;
	mowgli_dns_reslist_t *request;
	time_t next_time = 0;
	time_t timeout = 0;
	mowgli_dns_evloop_t *state = dns->dns_state;

	MOWGLI_ITER_FOREACH_SAFE(ptr, next_ptr, state->request_list.head)
	{
		request = ptr->data;
		timeout = request->sentat + request->timeout;

		if (now >= timeout)
		{
			if (--request->retries <= 0)
			{
				(*request->query->callback)(NULL, MOWGLI_DNS_RES_TIMEOUT, request->query->ptr);
				rem_request(dns, request);
				continue;
			}
			else
			{
				state->timeout_count[request->lastns]++;
				request->sentat = now;
				request->timeout += request->timeout;
				resend_query(dns, request);
			}
		}

		if ((next_time == 0) || (timeout < next_time))
			next_time = timeout;
	}

	return (next_time > now) ? next_time : (now + MOWGLI_DNS_AR_TTL);
}

/*
 * timeout_resolver - check request list
 */
static void
timeout_resolver(void *arg)
{
	mowgli_dns_t *dns = arg;
	mowgli_dns_evloop_t *state = dns->dns_state;
	time_t now, next;

	now = mowgli_eventloop_get_time(state->eventloop);
	next = timeout_query_list(dns, now);

	/* Reschedule */
	mowgli_timer_destroy(state->eventloop, state->timeout_resolver_timer);
	mowgli_timer_add(state->eventloop, "timeout_resolver", timeout_resolver, dns, next - now);
}

/*
 * mowgli_dns_evloop_add_local_domain - Add the domain to hostname, if it is missing
 * (as suggested by eps@TOASTER.SFSU.EDU)
 */
void
mowgli_dns_evloop_add_local_domain(mowgli_dns_t *dns, char *hname, size_t size)
{
	mowgli_dns_evloop_t *state = dns->dns_state;

	/* try to fix up unqualified names */
	if (strchr(hname, '.') == NULL)
		if (state->domain[0])
		{
			size_t len = strlen(hname);

			if ((strlen(state->domain) + len + 2) < size)
			{
				hname[len++] = '.';
				strcpy(hname + len, state->domain);
			}
		}
}

/*
 * rem_request - remove a request from the list.
 * This must also free any memory that has been allocated for
 * temporary storage of DNS results.
 */
static void
rem_request(mowgli_dns_t *dns, mowgli_dns_reslist_t *request)
{
	mowgli_dns_evloop_t *state = dns->dns_state;

	return_if_fail(request != NULL);

	mowgli_node_delete(&request->node, &state->request_list);
	mowgli_free(request->name);
	mowgli_heap_free(reslist_heap, request);
}

/*
 * make_request - Create a DNS request record for the server.
 */
static mowgli_dns_reslist_t *
make_request(mowgli_dns_t *dns, mowgli_dns_query_t *query)
{
	mowgli_dns_reslist_t *request = mowgli_heap_alloc(reslist_heap);
	mowgli_dns_evloop_t *state = dns->dns_state;

	request->sentat = mowgli_eventloop_get_time(state->eventloop);
	request->retries = 3;
	request->timeout = 4;	/* start at 4 and exponential inc. */
	request->query = query;

	mowgli_node_add(request, &request->node, &state->request_list);

	return request;
}

/*
 * mowgli_dns_evloop_delete_queries - cleanup outstanding queries
 * for which there no longer exist clients or conf lines.
 */
void
mowgli_dns_evloop_delete_queries(mowgli_dns_t *dns, const mowgli_dns_query_t *query)
{
	mowgli_node_t *ptr;
	mowgli_node_t *next_ptr;
	mowgli_dns_reslist_t *request;
	mowgli_dns_evloop_t *state = dns->dns_state;

	MOWGLI_ITER_FOREACH_SAFE(ptr, next_ptr, state->request_list.head)
	{
		if ((request = ptr->data) != NULL)
			if (query == request->query)
				rem_request(dns, request);
	}
}

/*
 * retryfreq - determine how many queries to wait before resending
 * if there have been that many consecutive timeouts
 */
static inline int
retryfreq(int timeouts)
{
	int i;
	int counter = 1;
	const int max_retries = 5;

	for (i = 0; i < (timeouts < max_retries ? timeouts : max_retries); i++)
		counter *= 3;

	return counter;
}

/*
 * send_res_msg - sends msg to a nameserver.
 * This should reflect /etc/resolv.conf.
 * Returns number of nameserver successfully sent to
 * or -1 if no successful sends.
 */
static int
send_res_msg(mowgli_dns_t *dns, const char *rmsg, int len, int rcount)
{
	int i;
	int ns;
	mowgli_dns_evloop_t *state = dns->dns_state;

	state->retrycnt++;

	/* First try a nameserver that seems to work. Every once in a while, try a possibly broken one
	 * to check if it is working again. */
	for (i = 0; i < state->nscount; i++)
	{
		ns = (i + rcount - 1) % state->nscount;

		if (state->timeout_count[ns] && state->retrycnt % retryfreq(state->timeout_count[ns]))
			continue;

		if (mowgli_vio_sendto(state->vio, rmsg, len, &state->nsaddr_list[ns]) == len)
			return ns;
	}

	/* No known working nameservers, try some broken one. */
	for (i = 0; i < state->nscount; i++)
	{
		ns = (i + rcount - 1) % state->nscount;

		if (!state->timeout_count[ns])
			continue;

		if (mowgli_vio_sendto(state->vio, rmsg, len, &state->nsaddr_list[ns]) == len)
			return ns;
	}

	return -1;
}

/*
 * find_id - find a dns request id (id is determined by dn_mkquery)
 */
static mowgli_dns_reslist_t *
find_id(mowgli_dns_t *dns, int id)
{
	mowgli_node_t *ptr;
	mowgli_dns_reslist_t *request;
	mowgli_dns_evloop_t *state = dns->dns_state;

	MOWGLI_ITER_FOREACH(ptr, state->request_list.head)
	{
		request = ptr->data;

		if (request->id == id)
			return request;
	}

	return NULL;
}

/*
 * mowgli_dns_evloop_gethost_byname - get host address from name
 *
 */
void
mowgli_dns_evloop_gethost_byname(mowgli_dns_t *dns, const char *name, mowgli_dns_query_t *query, int type)
{
	return_if_fail(name != NULL);

	do_query_name(dns, query, name, NULL, type);
}

/*
 * mowgli_dns_evloop_gethost_byaddr - get host name from address
 */
void
mowgli_dns_evloop_gethost_byaddr(mowgli_dns_t *dns, const struct sockaddr_storage *addr, mowgli_dns_query_t *query)
{
	return_if_fail(addr != NULL);

	do_query_number(dns, query, addr, NULL);
}

/*
 * do_query_name - nameserver lookup name
 */
static void
do_query_name(mowgli_dns_t *dns, mowgli_dns_query_t *query, const char *name, mowgli_dns_reslist_t *request, int type)
{
	char host_name[MOWGLI_DNS_RES_HOSTLEN + 1];

	mowgli_strlcpy(host_name, name, MOWGLI_DNS_RES_HOSTLEN + 1);
	mowgli_dns_evloop_add_local_domain(dns, host_name, MOWGLI_DNS_RES_HOSTLEN);

	if (request == NULL)
	{
		request = make_request(dns, query);
		request->name = mowgli_strdup(host_name);
	}

	mowgli_strlcpy(request->queryname, host_name, sizeof(request->queryname));
	request->type = type;
	query_name(dns, request);
}

/*
 * do_query_number - Use this to do reverse IP# lookups.
 */
static void
do_query_number(mowgli_dns_t *dns, mowgli_dns_query_t *query, const struct sockaddr_storage *addr, mowgli_dns_reslist_t *request)
{
	const unsigned char *cp;
	const size_t size = addr->ss_family == AF_INET ? sizeof(struct sockaddr_in) : sizeof(struct sockaddr_in6);

	if (request == NULL)
	{
		request = make_request(dns, query);
		memcpy(&request->addr, addr, size);
		request->name = (char *) mowgli_alloc(MOWGLI_DNS_RES_HOSTLEN + 1);
	}

	if (addr->ss_family == AF_INET)
	{
		const struct sockaddr_in *v4 = (const struct sockaddr_in *) addr;
		cp = (const unsigned char *) &v4->sin_addr.s_addr;

		sprintf(request->queryname, "%u.%u.%u.%u.in-addr.arpa", (unsigned int) (cp[3]),
			(unsigned int) (cp[2]), (unsigned int) (cp[1]), (unsigned int) (cp[0]));
	}
	else if (addr->ss_family == AF_INET6)
	{
		int i;
		char *rqptr = request->queryname;
		const struct sockaddr_in6 *v6 = (const struct sockaddr_in6 *) addr;
		cp = (const unsigned char *) &v6->sin6_addr.s6_addr;

		for (i = 15; i >= 0; i--, rqptr += 4)
			sprintf(rqptr, "%1x.%1x.",
				(unsigned int) (cp[i] & 0xf),
				(unsigned int) (cp[i] >> 4));

		strcpy(rqptr, ".ip6.arpa");
	}
	else
	{
		mowgli_log("do_query_number() called with invalid sockaddr_storage %d", addr->ss_family);
		return;
	}

	request->type = MOWGLI_DNS_T_PTR;
	query_name(dns, request);
}

/*
 * query_name - generate a query based on class, type and name.
 */
static void
query_name(mowgli_dns_t *dns, mowgli_dns_reslist_t *request)
{
	char buf[MOWGLI_DNS_MAXPACKET];
	int request_len = 0;
	int ns;
	mowgli_dns_evloop_t *state = dns->dns_state;

	memset(buf, 0, sizeof(buf));

	if ((request_len =
		     mowgli_dns_res_mkquery(request->queryname, MOWGLI_DNS_C_IN, request->type, (unsigned char *) buf,
					    sizeof(buf))) > 0)
	{
		mowgli_dns_resheader_t *header = (mowgli_dns_resheader_t *) buf;

		/*
		 * generate an unique id
		 * NOTE: we don't have to worry about converting this to and from
		 * network byte order, the nameserver does not interpret this value
		 * and returns it unchanged
		 */
		do
			header->id = (header->id + mowgli_random_int(state->rand)) & 0xffff;

		while (find_id(dns, header->id));

		request->id = header->id;
		++request->sends;

		ns = send_res_msg(dns, buf, request_len, request->sends);

		if (ns != -1)
			request->lastns = ns;
	}
}

static void
resend_query(mowgli_dns_t *dns, mowgli_dns_reslist_t *request)
{
	switch (request->type)
	{
	case MOWGLI_DNS_T_PTR:
		do_query_number(dns, NULL, &request->addr, request);
		break;
	case MOWGLI_DNS_T_A:
	case MOWGLI_DNS_T_AAAA:
		do_query_name(dns, NULL, request->name, request, request->type);
		break;
	default:
		break;
	}
}

/*
 * check_question - check if the reply really belongs to the
 * name we queried (to guard against late replies from previous
 * queries with the same id).
 */
static int
check_question(mowgli_dns_reslist_t *request, mowgli_dns_resheader_t *header, char *buf, char *eob)
{
	char hostbuf[MOWGLI_DNS_RES_HOSTLEN + 1];	/* working buffer */
	unsigned char *current;	/* current position in buf */
	int n;	/* temp count */

	current = (unsigned char *) buf + sizeof(mowgli_dns_resheader_t);

	if (header->qdcount != 1)
		return 0;

	n = mowgli_dns_dn_expand((unsigned char *) buf, (unsigned char *) eob, current, hostbuf, sizeof(hostbuf));

	if (n <= 0)
		return 0;

	if (strcasecmp(hostbuf, request->queryname))
		return 0;

	return 1;
}

/*
 * proc_answer - process name server reply
 */
static int
proc_answer(mowgli_dns_reslist_t *request, mowgli_dns_resheader_t *header, char *buf, char *eob)
{
	char hostbuf[MOWGLI_DNS_RES_HOSTLEN + 100];	/* working buffer */
	unsigned char *current;	/* current position in buf */
	int query_class;/* answer class */
	int type;	/* answer type */
	int n;	/* temp count */
	int rd_length;

	current = (unsigned char *) buf + sizeof(mowgli_dns_resheader_t);

	for (; header->qdcount > 0; --header->qdcount)
	{
		if ((n = mowgli_dns_dn_skipname(current, (unsigned char *) eob)) < 0)
			return 0;

		current += (size_t) n + MOWGLI_DNS_QFIXEDSIZE;
	}

	/* process each answer sent to us. */
	while (header->ancount > 0 && (char *) current < eob)
	{
		header->ancount--;

		n = mowgli_dns_dn_expand((unsigned char *) buf, (unsigned char *) eob, current, hostbuf,
					 sizeof(hostbuf));

		/* Broken message (< 0) or no more answers left (== 0) */
		if (n <= 0)
			return 0;

		hostbuf[MOWGLI_DNS_RES_HOSTLEN] = '\0';

		/* With Address arithmetic you have to be very anal -- this code was not working on alpha due
		 * to that (spotted by rodder/jailbird/dianora) */
		current += (size_t) n;

		if (!(((char *) current + MOWGLI_DNS_ANSWER_FIXED_SIZE) < eob))
			break;

		type = mowgli_dns_ns_get16(current);
		current += MOWGLI_DNS_TYPE_SIZE;

		query_class = mowgli_dns_ns_get16(current);
		current += MOWGLI_DNS_CLASS_SIZE;

		/* We may use this later at some point so... eliminate bogus GCC warning */
		(void) query_class;

		request->ttl = mowgli_dns_ns_get32(current);
		current += MOWGLI_DNS_TTL_SIZE;

		rd_length = mowgli_dns_ns_get16(current);
		current += MOWGLI_DNS_RDLENGTH_SIZE;

		/* Wait to set request->type until we verify this structure */
		switch (type)
		{
		case MOWGLI_DNS_T_A:
		{
			struct sockaddr_in *v4;

			if (request->type != MOWGLI_DNS_T_A)
				return 0;

			/*  check for invalid rd_length or too many addresses */
			if (rd_length != sizeof(struct in_addr))
				return 0;

			v4 = (struct sockaddr_in *) &request->addr;
			v4->sin_family = AF_INET;
			memcpy(&v4->sin_addr, current, sizeof(struct in_addr));

			return 1;
		}
		case MOWGLI_DNS_T_AAAA:
		{
			struct sockaddr_in6 *v6;

			if (request->type != MOWGLI_DNS_T_AAAA)
				return 0;

			if (rd_length != sizeof(struct in6_addr))
				return 0;

			v6 = (struct sockaddr_in6 *) &request->addr;
			v6->sin6_family = AF_INET6;
			memcpy(&v6->sin6_addr, current, sizeof(struct in6_addr));

			return 1;
		}
		case MOWGLI_DNS_T_PTR:

			if (request->type != MOWGLI_DNS_T_PTR)
				return 0;

			n = mowgli_dns_dn_expand((unsigned char *) buf, (unsigned char *) eob, current,
						 hostbuf, sizeof(hostbuf));

			/* Broken message or no more answers left */
			if (n <= 0)
				return 0;

			mowgli_strlcpy(request->name, hostbuf, MOWGLI_DNS_RES_HOSTLEN + 1);

			return 1;
		case MOWGLI_DNS_T_CNAME:

			/* real answer will follow */
			current += rd_length;
			break;
		default:

			/* XXX I'd rather just throw away the entire bogus thing but its possible its just a
			 * broken nameserver with still valid answers. But lets do some rudimentary logging for
			 * now... */
			mowgli_log("proc_answer(): bogus type %d", type);
			break;
		}
	}

	return 1;
}

/*
 * res_read_single_reply - read a dns reply from the nameserver and process it.
 * Return value: 1 if a packet was read, 0 otherwise
 */
static int
res_read_single_reply(mowgli_dns_t *dns)
{
	char buf[sizeof(mowgli_dns_resheader_t) + MOWGLI_DNS_MAXPACKET]

	/* Sparc and alpha need 16bit-alignment for accessing header->id (which is uint16_t).
	   Because of the header = (mowgli_dns_resheader_t*) buf; later on, this is neeeded. --FaUl */
#if defined(__sparc__) || defined(__alpha__)
	__attribute__((aligned(16)))
#endif
	;
	mowgli_dns_resheader_t *header;
	mowgli_dns_reslist_t *request = NULL;
	mowgli_dns_reply_t *reply = NULL;
	int rc;
	int answer_count;
	mowgli_vio_sockaddr_t lsin;
	mowgli_dns_evloop_t *state = dns->dns_state;

	rc = mowgli_vio_recvfrom(state->vio, buf, sizeof(buf), &lsin);

	/* No packet */
	if ((rc == 0) || (rc == -1))
		return 0;

	/* Too small */
	if (rc <= (int) (sizeof(mowgli_dns_resheader_t)))
		return 1;

	/*
	 * convert DNS reply reader from Network byte order to CPU byte order.
	 */
	header = (mowgli_dns_resheader_t *) buf;
	header->ancount = ntohs(header->ancount);
	header->qdcount = ntohs(header->qdcount);
	header->nscount = ntohs(header->nscount);
	header->arcount = ntohs(header->arcount);

	/* response for an id which we have already received an answer for
	 * just ignore this response. */
	if ((request = find_id(dns, header->id)) == 0)
		return 1;

	/* check against possibly fake replies */
	if (!res_ourserver(dns, &lsin.addr))
		return 1;

	if (!check_question(request, header, buf, buf + rc))
		return 1;

	if ((header->rcode != MOWGLI_DNS_NO_ERRORS) || (header->ancount == 0))
	{
		if (header->rcode == MOWGLI_DNS_NXDOMAIN)
		{
			(*request->query->callback)(NULL, MOWGLI_DNS_RES_NXDOMAIN, request->query->ptr);
			rem_request(dns, request);
		}
		else
		{
			/*
			 * If a bad error was returned, we stop here and dont send
			 * send any more (no retries granted).
			 */
			(*request->query->callback)(NULL, MOWGLI_DNS_RES_INVALID, request->query->ptr);
			rem_request(dns, request);
		}

		return 1;
	}

	/* If this fails there was an error decoding the received packet,
	 * give up. -- jilles
	 */
	answer_count = proc_answer(request, header, buf, buf + rc);

	if (answer_count)
	{
		if (request->type == MOWGLI_DNS_T_PTR)
		{
			if (request->name == NULL)
			{
				/* got a PTR response with no name, something bogus is happening
				 * don't bother trying again, the client address doesn't resolve
				 */
				(*request->query->callback)(reply, MOWGLI_DNS_RES_INVALID, request->query->ptr);
				rem_request(dns, request);
				return 1;
			}

			/* Lookup the 'authoritative' name that we were given for the
			 * ip#. */
			if (request->addr.ss_family == AF_INET6)
				mowgli_dns_evloop_gethost_byname(dns, request->name, request->query, MOWGLI_DNS_T_AAAA);
			else
				mowgli_dns_evloop_gethost_byname(dns, request->name, request->query, MOWGLI_DNS_T_A);

			rem_request(dns, request);
		}
		else
		{
			/* got a name and address response, client resolved */
			reply = make_dnsreply(request);
			(*request->query->callback)(reply, MOWGLI_DNS_RES_SUCCESS, request->query->ptr);
			mowgli_free(reply);
			rem_request(dns, request);
		}
	}
	else
	{
		/* couldn't decode, give up -- jilles */
		(*request->query->callback)(NULL, MOWGLI_DNS_RES_INVALID, request->query->ptr);
		rem_request(dns, request);
	}

	return 1;
}

static mowgli_dns_reply_t *
make_dnsreply(mowgli_dns_reslist_t *request)
{
	mowgli_dns_reply_t *cp;

	return_val_if_fail(request != 0, NULL);

	cp = (mowgli_dns_reply_t *) mowgli_alloc(sizeof(mowgli_dns_reply_t));

	cp->h_name = request->name;
	memcpy(&cp->addr, &request->addr, sizeof(cp->addr));
	return cp;
}

static void
res_readreply(mowgli_eventloop_t *eventloop, mowgli_eventloop_io_t *io, mowgli_eventloop_io_dir_t dir, void *userdata)
{
	mowgli_dns_t *dns = userdata;

	while (res_read_single_reply(dns))
	{ }
}

/* DNS ops for this resolver */
const mowgli_dns_ops_t mowgli_dns_evloop_resolver =
{
	.mowgli_dns_init_func_t = mowgli_dns_evloop_init,
	.mowgli_dns_fini_func_t = mowgli_dns_evloop_destroy,
	.mowgli_dns_restart_func_t = mowgli_dns_evloop_restart,
	.mowgli_dns_delete_query_func_t = mowgli_dns_evloop_delete_queries,
	.mowgli_dns_gethost_byname_func_t = mowgli_dns_evloop_gethost_byname,
	.mowgli_dns_gethost_byaddr_func_t = mowgli_dns_evloop_gethost_byaddr,
};
