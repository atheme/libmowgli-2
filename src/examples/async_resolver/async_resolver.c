/* This code is in the public domain. */

#include <mowgli.h>

typedef struct
{
	char *domain;
	mowgli_dns_query_t query;
} dns_query;

static void
resolve_cb(mowgli_dns_reply_t *reply, int reason, void *vptr)
{
	char buf[2048];
	dns_query *dnsquery = vptr;
	const void *sockptr;

	if (reply == NULL)
	{
		printf("Got null reply for %s\n", dnsquery->domain);

		switch (reason)
		{
		case MOWGLI_DNS_RES_NXDOMAIN:
			printf("Nonexistent domain\n");
			break;
		case MOWGLI_DNS_RES_INVALID:
			printf("Invalid domain\n");
			break;
		case MOWGLI_DNS_RES_TIMEOUT:
			printf("Timed out\n");
			break;
		}

		goto end;
	}

	printf("Finished %s\n", dnsquery->domain);
	printf("Hostname: %s\n", reply->h_name);

	if (reply->addr.addr.ss_family == AF_INET)
	{
		const struct sockaddr_in *saddr = (const struct sockaddr_in *) &reply->addr.addr;
		sockptr = &saddr->sin_addr;
	}
	else if (reply->addr.addr.ss_family == AF_INET6)
	{
		const struct sockaddr_in6 *saddr = (const struct sockaddr_in6 *) &reply->addr.addr;
		sockptr = &saddr->sin6_addr;
	}
	else
	{
		printf("Invalid Address family %d\n", reply->addr.addr.ss_family);
		return;
	}

	inet_ntop(reply->addr.addr.ss_family, sockptr, buf, sizeof(buf));
	printf("Resolved: %s\n", buf);

end:
	mowgli_free(dnsquery->domain);
	mowgli_free(vptr);
}

static void
read_data(mowgli_eventloop_t *eventloop, mowgli_eventloop_io_t *io, mowgli_eventloop_io_dir_t dir, void *userdata)
{
	mowgli_eventloop_pollable_t *pollable = mowgli_eventloop_io_pollable(io);
	mowgli_dns_t *dns = userdata;
	char buf[2048];
	char *ch;
	int ret;

	return_if_fail(pollable->fd == STDIN_FILENO);

	if ((ret = read(pollable->fd, buf, sizeof(buf))) < 0)
	{
		perror("read");
		mowgli_pollable_destroy(eventloop, io);
		return;
	}
	else if (ret == 0)
	{
		return;
	}

	buf[--ret] = '\0';

	ch = strtok(buf, " ");

	while (ch != NULL)
	{
		dns_query *dnsquery = mowgli_alloc(sizeof(dns_query));
		mowgli_dns_query_t *query = &dnsquery->query;

		printf("Domain input: %s\n", ch);
		printf("End domain input\n");

		query->callback = resolve_cb;
		query->ptr = dnsquery;
		dnsquery->domain = mowgli_strdup(ch);

		if (*ch == '+')
		{
			int type;
			void *addrptr;
			struct sockaddr_storage addr;

			if (strchr(++ch, ':') != NULL)
			{
				struct sockaddr_in6 *saddr = (struct sockaddr_in6 *) &addr;
				type = AF_INET6;
				addrptr = &saddr->sin6_addr;
			}
			else
			{
				struct sockaddr_in *saddr = (struct sockaddr_in *) &addr;
				type = AF_INET;
				addrptr = &saddr->sin_addr;
			}

			addr.ss_family = type;

			if ((ret = inet_pton(type, ch, addrptr)) != 1)
			{
				if (ret == -1)
					perror("inet_pton");
				else
					printf("Invalid address %s\n", ch);

				return;
			}

			mowgli_dns_gethost_byaddr(dns, &addr, query);
		}
		else
		{
			mowgli_dns_gethost_byname(dns, ch, query, MOWGLI_DNS_T_A);
		}

		dnsquery->domain = mowgli_strdup(ch);
		ch = strtok(NULL, " ");
	}
}

int
main(void)
{
	mowgli_eventloop_t *evloop = mowgli_eventloop_create();
	mowgli_dns_t *dns = mowgli_dns_create(evloop, MOWGLI_DNS_TYPE_ASYNC);
	mowgli_eventloop_pollable_t *stdin_pollable = mowgli_pollable_create(evloop, STDIN_FILENO, dns);

	mowgli_pollable_set_nonblocking(stdin_pollable, true);

	mowgli_pollable_setselect(evloop, stdin_pollable, MOWGLI_EVENTLOOP_IO_READ, read_data);

	mowgli_eventloop_run(evloop);

	mowgli_eventloop_destroy(evloop);

	return 0;
}
