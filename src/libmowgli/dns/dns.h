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

#ifndef __MOWGLI_DNS_DNS_H__
#define __MOWGLI_DNS_DNS_H__

/* Longest hostname we're willing to work with */
#define MOWGLI_DNS_RES_HOSTLEN 512

/* Resolver types */
#define MOWGLI_DNS_TYPE_CUSTOM 0
#define MOWGLI_DNS_TYPE_ASYNC 1
#define MOWGLI_DNS_TYPE_HELPER 2

/* Lookup types */
#define MOWGLI_DNS_T_A 1
#define MOWGLI_DNS_T_AAAA 28
#define MOWGLI_DNS_T_PTR 12
#define MOWGLI_DNS_T_CNAME 5
#define MOWGLI_DNS_T_MX 15
#define MOWGLI_DNS_T_TXT 16
#define MOWGLI_DNS_T_SSHFP 44
#define MOWGLI_DNS_T_NULL 10

/* Return codes */
#define MOWGLI_DNS_RES_SUCCESS 0
#define MOWGLI_DNS_RES_NXDOMAIN 1
#define MOWGLI_DNS_RES_INVALID 2
#define MOWGLI_DNS_RES_TIMEOUT 3

typedef struct _mowgli_dns_t mowgli_dns_t;
typedef struct _mowgli_dns_query_t mowgli_dns_query_t;
typedef struct _mowgli_dns_reply_t mowgli_dns_reply_t;

typedef struct
{
	int (*mowgli_dns_init_func_t)(mowgli_dns_t *, mowgli_eventloop_t *);
	void (*mowgli_dns_fini_func_t)(mowgli_dns_t *);
	int (*mowgli_dns_restart_func_t)(mowgli_dns_t *);
	void (*mowgli_dns_delete_query_func_t)(mowgli_dns_t *, const mowgli_dns_query_t *);
	void (*mowgli_dns_gethost_byname_func_t)(mowgli_dns_t *, const char *, mowgli_dns_query_t *, int);
	void (*mowgli_dns_gethost_byaddr_func_t)(mowgli_dns_t *, const struct sockaddr_storage *, mowgli_dns_query_t *);
} mowgli_dns_ops_t;

struct _mowgli_dns_reply_t
{
	char *h_name;
	mowgli_vio_sockaddr_t addr;
};

struct _mowgli_dns_t
{
	int dns_type;
	const mowgli_dns_ops_t *dns_ops;
	void *dns_state;
};

struct _mowgli_dns_query_t
{
	void *ptr;	/* pointer used by callback to identify request */
	void (*callback)(mowgli_dns_reply_t *reply, int result, void *vptr);	/* callback to call */
};

extern mowgli_dns_t *mowgli_dns_create(mowgli_eventloop_t *eventloop, int implementation);
extern int mowgli_dns_init(mowgli_dns_t *dns, mowgli_eventloop_t *eventloop, const mowgli_dns_ops_t *ops);
extern void mowgli_dns_destroy(mowgli_dns_t *dns);
extern int mowgli_dns_restart(mowgli_dns_t *dns);
extern void mowgli_dns_delete_query(mowgli_dns_t *dns, const mowgli_dns_query_t *query);
extern void mowgli_dns_gethost_byname(mowgli_dns_t *dns, const char *name, mowgli_dns_query_t *query, int type);
extern void mowgli_dns_gethost_byaddr(mowgli_dns_t *dns, const struct sockaddr_storage *addr, mowgli_dns_query_t *query);

/* Pull in headers that depend on these types */
#include "evloop_res.h"
#include "evloop_reslib.h"

#endif
