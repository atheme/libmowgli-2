/*
 * dns/reslib.h
 *
 * $Id: reslib.h 446 2006-02-12 02:46:54Z db $
 */

#ifndef __MOWGLI_DNS_RESLIB_H__
#define __MOWGLI_DNS_RESLIB_H__

/* Here we define some values lifted from nameser.h */
#define MOWGLI_DNS_NS_NOTIFY_OP 4
#define MOWGLI_DNS_NS_INT16SIZE 2
#define MOWGLI_DNS_NS_IN6ADDRSIZE 16
#define MOWGLI_DNS_NS_INADDRSIZE 4
#define MOWGLI_DNS_NS_INT32SIZE 4
#define MOWGLI_DNS_NS_CMPRSFLAGS 0xc0
#define MOWGLI_DNS_NS_MAXCDNAME 255
#define MOWGLI_DNS_QUERY 0
#define MOWGLI_DNS_IQUERY 1
#define MOWGLI_DNS_NO_ERRORS 0
#define MOWGLI_DNS_SERVFAIL 2
#define MOWGLI_DNS_NXDOMAIN 3
#define MOWGLI_DNS_C_IN 1
#define MOWGLI_DNS_QFIXEDSIZE 4
#define MOWGLI_DNS_RRFIXEDSIZE 10
#define MOWGLI_DNS_HFIXEDSIZE 12

typedef struct
{
	unsigned id : 16;	/* query identification number */
#ifdef WORDS_BIGENDIAN

	/* fields in third byte */
	unsigned qr : 1;/* response flag */
	unsigned opcode : 4;	/* purpose of message */
	unsigned aa : 1;/* authoritive answer */
	unsigned tc : 1;/* truncated message */
	unsigned rd : 1;/* recursion desired */
	/* fields in fourth byte */
	unsigned ra : 1;/* recursion available */
	unsigned unused : 1;	/* unused bits (MBZ as of 4.9.3a3) */
	unsigned ad : 1;/* authentic data from named */
	unsigned cd : 1;/* checking disabled by resolver */
	unsigned rcode : 4;	/* response code */
#else

	/* fields in third byte */
	unsigned rd : 1;/* recursion desired */
	unsigned tc : 1;/* truncated message */
	unsigned aa : 1;/* authoritive answer */
	unsigned opcode : 4;	/* purpose of message */
	unsigned qr : 1;/* response flag */
	/* fields in fourth byte */
	unsigned rcode : 4;	/* response code */
	unsigned cd : 1;/* checking disabled by resolver */
	unsigned ad : 1;/* authentic data from named */
	unsigned unused : 1;	/* unused bits (MBZ as of 4.9.3a3) */
	unsigned ra : 1;/* recursion available */
#endif

	/* remaining bytes */
	unsigned qdcount : 16;	/* number of question entries */
	unsigned ancount : 16;	/* number of answer entries */
	unsigned nscount : 16;	/* number of authority entries */
	unsigned arcount : 16;	/* number of resource entries */
} mowgli_dns_resheader_t;

/*
 * Inline versions of get/put short/long.  Pointer is advanced.
 */
#define MOWGLI_DNS_NS_GET16(s, cp) { \
		const unsigned char *t_cp = (const unsigned char *) (cp); \
		(s) = ((uint16_t) t_cp[0] << 8)	\
		      | ((uint16_t) t_cp[1]) \
		; \
		(cp) += MOWGLI_DNS_NS_INT16SIZE; \
}

#define MOWGLI_DNS_NS_GET32(l, cp) { \
		const unsigned char *t_cp = (const unsigned char *) (cp); \
		(l) = ((uint32_t) t_cp[0] << 24) \
		      | ((uint32_t) t_cp[1] << 16) \
		      | ((uint32_t) t_cp[2] << 8) \
		      | ((uint32_t) t_cp[3]) \
		; \
		(cp) += MOWGLI_DNS_NS_INT32SIZE; \
}

#define MOWGLI_DNS_NS_PUT16(s, cp) { \
		uint16_t t_s = (uint16_t) (s); \
		unsigned char *t_cp = (unsigned char *) (cp); \
		*t_cp++ = t_s >> 8; \
		*t_cp = t_s; \
		(cp) += MOWGLI_DNS_NS_INT16SIZE; \
}

#define MOWGLI_DNS_NS_PUT32(l, cp) { \
		uint32_t t_l = (uint32_t) (l); \
		unsigned char *t_cp = (unsigned char *) (cp); \
		*t_cp++ = t_l >> 24; \
		*t_cp++ = t_l >> 16; \
		*t_cp++ = t_l >> 8; \
		*t_cp = t_l; \
		(cp) += MOWGLI_DNS_NS_INT32SIZE; \
}

extern int mowgli_dns_dn_expand(const unsigned char *msg, const unsigned char *eom, const unsigned char *src, char *dst, int dstsiz);
extern int mowgli_dns_dn_skipname(const unsigned char *ptr, const unsigned char *eom);
extern unsigned int mowgli_dns_ns_get16(const unsigned char *src);
extern unsigned long mowgli_dns_ns_get32(const unsigned char *src);
extern void mowgli_dns_ns_put16(unsigned int src, unsigned char *dst);
extern void mowgli_dns_ns_put32(unsigned long src, unsigned char *dst);
extern int mowgli_dns_res_mkquery(const char *dname, int query_class, int type, unsigned char *buf, int buflen);

#endif
