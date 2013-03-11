/*
 * Copyright (c) 2012 Elizabeth J. Myers. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "mowgli.h"

mowgli_dns_t *
mowgli_dns_create(mowgli_eventloop_t *eventloop, int implementation)
{
	mowgli_dns_t *dns = mowgli_alloc(sizeof(mowgli_dns_t));
	const mowgli_dns_ops_t *ops;

	switch (implementation)
	{
	case MOWGLI_DNS_TYPE_CUSTOM:
		return dns;
	case MOWGLI_DNS_TYPE_ASYNC:
	default:
		ops = &mowgli_dns_evloop_resolver;
		break;
	}

	if (mowgli_dns_init(dns, eventloop, ops) != 0)
	{
		mowgli_free(dns);
		return NULL;
	}

	return dns;
}

int
mowgli_dns_init(mowgli_dns_t *dns, mowgli_eventloop_t *eventloop, const mowgli_dns_ops_t *ops)
{
	return_val_if_fail(dns != NULL, -1);

	dns->dns_ops = ops;

	return dns->dns_ops->mowgli_dns_init_func_t(dns, eventloop);
}

void
mowgli_dns_destroy(mowgli_dns_t *dns)
{
	dns->dns_ops->mowgli_dns_fini_func_t(dns);

	mowgli_free(dns);
}

int
mowgli_dns_restart(mowgli_dns_t *dns)
{
	return dns->dns_ops->mowgli_dns_restart_func_t(dns);
}

void
mowgli_dns_delete_query(mowgli_dns_t *dns, const mowgli_dns_query_t *query)
{
	dns->dns_ops->mowgli_dns_delete_query_func_t(dns, query);
}

void
mowgli_dns_gethost_byname(mowgli_dns_t *dns, const char *name, mowgli_dns_query_t *query, int type)
{
	dns->dns_ops->mowgli_dns_gethost_byname_func_t(dns, name, query, type);
}

void
mowgli_dns_gethost_byaddr(mowgli_dns_t *dns, const struct sockaddr_storage *addr, mowgli_dns_query_t *query)
{
	dns->dns_ops->mowgli_dns_gethost_byaddr_func_t(dns, addr, query);
}
