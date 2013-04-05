/*
 * Copyright (c) 1985, 1993
 *    The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*
 * Portions Copyright (c) 1993 by Digital Equipment Corporation.
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies, and that
 * the name of Digital Equipment Corporation not be used in advertising or
 * publicity pertaining to distribution of the document or software without
 * specific, written prior permission.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND DIGITAL EQUIPMENT CORP. DISCLAIMS ALL
 * WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS.   IN NO EVENT SHALL DIGITAL EQUIPMENT
 * CORPORATION BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS
 * ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 */

/*
 * Portions Copyright (c) 1996-1999 by Internet Software Consortium.
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND INTERNET SOFTWARE CONSORTIUM DISCLAIMS
 * ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL INTERNET SOFTWARE
 * CONSORTIUM BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS
 * ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 */

/* Original copyright ISC as above. Code modified specifically for ircd use from the following
 * orginal files in bind ... res_comp.c ns_name.c ns_netint.c res_init.c - Dianora */

#include "mowgli.h"

#ifdef _WIN32
# define EMSGSIZE WSAEMSGSIZE
#endif

#define MOWGLI_DNS_NS_TYPE_ELT 0x40	/* EDNS0 extended label type */
#define MOWGLI_DNS_LABELTYPE_BITSTRING 0x41

/* from Hybrid Id: reslib.c 177 2005-10-22 09:05:05Z michael $ */

static const char digitvalue[256] =
{
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,	/* 16 */
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,	/* 32 */
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,	/* 48 */
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, -1, -1, -1, -1, -1, -1,	/* 64 */
	-1, 10, 11, 12, 13, 14, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1,	/* 80 */
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,	/* 96 */
	-1, 10, 11, 12, 13, 14, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1,	/* 112 */
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,	/* 128 */
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,	/* 256 */
};

static const char digits[] = "0123456789";

static int labellen(const unsigned char *lp);
static bool mowgli_dns_is_special(int ch);
static bool mowgli_dns_is_printable(int ch);
static int mowgli_dns_decode_bitstring(const char **cpp, char *dn, const char *eom);
static int mowgli_dns_ns_name_compress(const char *src, unsigned char *dst, size_t dstsiz, unsigned char **dnptrs, unsigned char **lastdnptr);
static int mowgli_dns_dn_find(const unsigned char *, const unsigned char *, const unsigned char *const *, const unsigned char *const *);
static int mowgli_dns_encode_bitsring(const char **, const char *, unsigned char **, unsigned char **, const char *);
static int mowgli_dns_ns_name_uncompress(const unsigned char *, const unsigned char *, const unsigned char *, char *, size_t);
static int mowgli_dns_ns_name_unpack(const unsigned char *, const unsigned char *, const unsigned char *, unsigned char *, size_t);
static int mowgli_dns_ns_name_ntop(const char *, char *, size_t);
static int mowgli_dns_ns_name_skip(const unsigned char **, const unsigned char *);
static int mowgli_dns_mklower(int ch);

/*
 * Expand compressed domain name 'comp_dn' to full domain name.
 * 'rmsg' is a pointer to the begining of the message,
 * 'eomorig' points to the first location after the message,
 * 'exp_dn' is a pointer to a buffer of size 'length' for the result.
 * Return size of compressed name or -1 if there was an error.
 */
int
mowgli_dns_dn_expand(const unsigned char *rmsg, const unsigned char *eom, const unsigned char *src, char *dst, int dstsiz)
{
	int n = mowgli_dns_ns_name_uncompress(rmsg, eom, src, dst, (size_t) dstsiz);

	if ((n > 0) && (dst[0] == '.'))
		dst[0] = '\0';

	return n;
}

/*
 * mowgli_dns_ns_name_uncompress(rmsg, eom, src, dst, dstsiz)
 *  Expand compressed domain name to presentation format.
 * return:
 *  Number of bytes read out of `src', or -1 (with errno set).
 * note:
 *  Root domain returns as "." not "".
 */
static int
mowgli_dns_ns_name_uncompress(const unsigned char *rmsg, const unsigned char *eom, const unsigned char *src, char *dst, size_t dstsiz)
{
	unsigned char tmp[MOWGLI_DNS_NS_MAXCDNAME];
	int n;

	if ((n = mowgli_dns_ns_name_unpack(rmsg, eom, src, tmp, sizeof tmp)) == -1)
		return -1;

	if (mowgli_dns_ns_name_ntop((char *) tmp, dst, dstsiz) == -1)
		return -1;

	return n;
}

/*
 * mowgli_dns_ns_name_unpack(rmsg, eom, src, dst, dstsiz)
 *  Unpack a domain name from a message, source may be compressed.
 * return:
 *  -1 if it fails, or consumed octets if it succeeds.
 */
static int
mowgli_dns_ns_name_unpack(const unsigned char *rmsg, const unsigned char *eom, const unsigned char *src, unsigned char *dst, size_t dstsiz)
{
	const unsigned char *srcp, *dstlim;
	unsigned char *dstp;
	int n, len, checked, l;

	len = -1;
	checked = 0;
	dstp = dst;
	srcp = src;
	dstlim = dst + dstsiz;

	if ((srcp < rmsg) || (srcp >= eom))
	{
		errno = EMSGSIZE;
		return -1;
	}

	/* Fetch next label in domain name. */
	while ((n = *srcp++) != 0)
	{
		/* Check for indirection. */
		switch (n & MOWGLI_DNS_NS_CMPRSFLAGS)
		{
		case 0:
		case MOWGLI_DNS_NS_TYPE_ELT:

			/* Limit checks. */
			if ((l = labellen(srcp - 1)) < 0)
			{
				errno = EMSGSIZE;
				return -1;
			}

			if ((dstp + l + 1 >= dstlim) || (srcp + l >= eom))
			{
				errno = EMSGSIZE;
				return -1;
			}

			checked += l + 1;
			*dstp++ = n;
			memcpy(dstp, srcp, l);
			dstp += l;
			srcp += l;
			break;

		case MOWGLI_DNS_NS_CMPRSFLAGS:

			if (srcp >= eom)
			{
				errno = EMSGSIZE;
				return -1;
			}

			if (len < 0)
				len = srcp - src + 1;

			srcp = rmsg + (((n & 0x3f) << 8) | (*srcp & 0xff));

			if ((srcp < rmsg) || (srcp >= eom))	/* Out of range. */
			{
				errno = EMSGSIZE;
				return -1;
			}

			checked += 2;

			/*
			 * Check for loops in the compressed name;
			 * if we've looked at the whole message,
			 * there must be a loop.
			 */
			if (checked >= eom - rmsg)
			{
				errno = EMSGSIZE;
				return -1;
			}

			break;

		default:
			errno = EMSGSIZE;
			return -1;	/* flag error */
		}
	}

	*dstp = '\0';

	if (len < 0)
		len = srcp - src;

	return len;
}

/*
 * mowgli_dns_ns_name_ntop(src, dst, dstsiz)
 *  Convert an encoded domain name to mowgli_dns_is_printable ascii as per RFC1035.
 * return:
 *  Number of bytes written to buffer, or -1 (with errno set)
 * notes:
 *  The root is returned as "."
 *  All other domains are returned in non absolute form
 */
static int
mowgli_dns_ns_name_ntop(const char *src, char *dst, size_t dstsiz)
{
	const char *cp;
	char *dn, *eom;
	unsigned char c;
	unsigned int n;
	int l;

	cp = src;
	dn = dst;
	eom = dst + dstsiz;

	while ((n = *cp++) != 0)
	{
		if ((n & MOWGLI_DNS_NS_CMPRSFLAGS) == MOWGLI_DNS_NS_CMPRSFLAGS)
		{
			/* Some kind of compression pointer. */
			errno = EMSGSIZE;
			return -1;
		}

		if (dn != dst)
		{
			if (dn >= eom)
			{
				errno = EMSGSIZE;
				return -1;
			}

			*dn++ = '.';
		}

		if ((l = labellen((const unsigned char *) (cp - 1))) < 0)
		{
			errno = EMSGSIZE;	/* XXX */
			return -1;
		}

		if (dn + l >= eom)
		{
			errno = EMSGSIZE;
			return -1;
		}

		if ((n & MOWGLI_DNS_NS_CMPRSFLAGS) == MOWGLI_DNS_NS_TYPE_ELT)
		{
			int m;

			if (n != MOWGLI_DNS_LABELTYPE_BITSTRING)
			{
				/* XXX: labellen should reject this case */
				errno = EINVAL;
				return -1;
			}

			if ((m = mowgli_dns_decode_bitstring(&cp, dn, eom)) < 0)
			{
				errno = EMSGSIZE;
				return -1;
			}

			dn += m;
			continue;
		}

		for ((void) NULL; l > 0; l--)
		{
			c = *cp++;

			if (mowgli_dns_is_special(c))
			{
				if (dn + 1 >= eom)
				{
					errno = EMSGSIZE;
					return -1;
				}

				*dn++ = '\\';
				*dn++ = (char) c;
			}
			else if (!mowgli_dns_is_printable(c))
			{
				if (dn + 3 >= eom)
				{
					errno = EMSGSIZE;
					return -1;
				}

				*dn++ = '\\';
				*dn++ = digits[c / 100];
				*dn++ = digits[(c % 100) / 10];
				*dn++ = digits[c % 10];
			}
			else
			{
				if (dn >= eom)
				{
					errno = EMSGSIZE;
					return -1;
				}

				*dn++ = (char) c;
			}
		}
	}

	if (dn == dst)
	{
		if (dn >= eom)
		{
			errno = EMSGSIZE;
			return -1;
		}

		*dn++ = '.';
	}

	if (dn >= eom)
	{
		errno = EMSGSIZE;
		return -1;
	}

	*dn++ = '\0';
	return dn - dst;
}

/*
 * Pack domain name 'exp_dn' in presentation form into 'comp_dn'.
 * Return the size of the compressed name or -1.
 * 'length' is the size of the array pointed to by 'comp_dn'.
 */
static int
mowgli_dns_dn_comp(const char *src, unsigned char *dst, int dstsiz, unsigned char **dnptrs, unsigned char **lastdnptr)
{
	return mowgli_dns_ns_name_compress(src, dst, (size_t) dstsiz, dnptrs, lastdnptr);
}

/*
 * Skip over a compressed domain name. Return the size or -1.
 */
int
mowgli_dns_dn_skipname(const unsigned char *ptr, const unsigned char *eom)
{
	const unsigned char *saveptr = ptr;

	if (mowgli_dns_ns_name_skip(&ptr, eom) == -1)
		return -1;

	return ptr - saveptr;
}

/*
 * ns_name_skip(ptrptr, eom)
 *  Advance *ptrptr to skip over the compressed name it points at.
 * return:
 *  0 on success, -1 (with errno set) on failure.
 */
static int
mowgli_dns_ns_name_skip(const unsigned char **ptrptr, const unsigned char *eom)
{
	const unsigned char *cp;
	unsigned int n;
	int l;

	cp = *ptrptr;

	while (cp < eom && (n = *cp++) != 0)
	{
		/* Check for indirection. */
		switch (n & MOWGLI_DNS_NS_CMPRSFLAGS)
		{
		case 0:	/* normal case, n == len */
			cp += n;
			continue;
		case MOWGLI_DNS_NS_TYPE_ELT:	/* EDNS0 extended label */

			if ((l = labellen(cp - 1)) < 0)
			{
				errno = EMSGSIZE;	/* XXX */
				return -1;
			}

			cp += l;
			continue;
		case MOWGLI_DNS_NS_CMPRSFLAGS:	/* indirection */
			cp++;
			break;
		default:/* illegal type */
			errno = EMSGSIZE;
			return -1;
		}

		break;
	}

	if (cp > eom)
	{
		errno = EMSGSIZE;
		return -1;
	}

	*ptrptr = cp;
	return 0;
}

unsigned int
mowgli_dns_ns_get16(const unsigned char *src)
{
	unsigned int dst;

	MOWGLI_DNS_NS_GET16(dst, src);
	return dst;
}

unsigned long
mowgli_dns_ns_get32(const unsigned char *src)
{
	unsigned long dst;

	MOWGLI_DNS_NS_GET32(dst, src);
	return dst;
}

void
mowgli_dns_ns_put16(unsigned int src, unsigned char *dst)
{
	MOWGLI_DNS_NS_PUT16(src, dst);
}

void
mowgli_dns_ns_put32(unsigned long src, unsigned char *dst)
{
	MOWGLI_DNS_NS_PUT32(src, dst);
}

/* From ns_name.c */

/*
 * mowgli_dns_is_special(ch)
 *      Thinking in noninternationalized USASCII (per the DNS spec),
 *      is this characted mowgli_dns_is_special ("in need of quoting") ?
 * return:
 *      boolean.
 */
static bool
mowgli_dns_is_special(int ch)
{
	switch (ch)
	{
	case 0x22:	/* '"' */
	case 0x2E:	/* '.' */
	case 0x3B:	/* ';' */
	case 0x5C:	/* '\\' */
	case 0x28:	/* '(' */
	case 0x29:	/* ')' */
	/* Special modifiers in zone files. */
	case 0x40:	/* '@' */
	case 0x24:	/* '$' */
		return true;
	default:
		return false;
	}
}

static int
labellen(const unsigned char *lp)
{
	int bitlen;
	unsigned char l = *lp;

	if ((l & MOWGLI_DNS_NS_CMPRSFLAGS) == MOWGLI_DNS_NS_CMPRSFLAGS)
		/* should be avoided by the caller */
		return -1;

	if ((l & MOWGLI_DNS_NS_CMPRSFLAGS) == MOWGLI_DNS_NS_TYPE_ELT)
	{
		if (l == MOWGLI_DNS_LABELTYPE_BITSTRING)
		{
			if ((bitlen = *(lp + 1)) == 0)
				bitlen = 256;

			return (bitlen + 7) / 8 + 1;
		}

		return -1;	/* unknwon ELT */
	}

	return l;
}

/*
 * mowgli_dns_is_printable(ch)
 *      Thinking in noninternationalized USASCII (per the DNS spec),
 *      is this character visible and not a space when printed ?
 * return:
 *      boolean.
 */
static bool
mowgli_dns_is_printable(int ch)
{
	return (ch > 0x20 && ch < 0x7f) ? true : false;
}

static int
mowgli_dns_decode_bitstring(const char **cpp, char *dn, const char *eom)
{
	const char *cp = *cpp;
	char *beg = dn, tc;
	int b, blen, plen;

	if ((blen = (*cp & 0xff)) == 0)
		blen = 256;

	plen = (blen + 3) / 4;
	plen += sizeof("\\[x/]") + (blen > 99 ? 3 : (blen > 9) ? 2 : 1);

	if (dn + plen >= eom)
		return -1;

	cp++;
	dn += sprintf(dn, "\\[x");

	for (b = blen; b > 7; b -= 8, cp++)
		dn += sprintf(dn, "%02x", *cp & 0xff);

	if (b > 4)
	{
		tc = *cp++;
		dn += sprintf(dn, "%02x", tc & (0xff << (8 - b)));
	}
	else if (b > 0)
	{
		tc = *cp++;
		dn += sprintf(dn, "%1x", ((tc >> 4) & 0x0f) & (0x0f << (4 - b)));
	}

	dn += sprintf(dn, "/%d]", blen);

	*cpp = cp;
	return dn - beg;
}

/*
 * mowgli_dns_ns_name_pton(src, dst, dstsiz)
 *  Convert a ascii string into an encoded domain name as per RFC1035.
 * return:
 *  -1 if it fails
 *  1 if string was fully qualified
 *  0 is string was not fully qualified
 * notes:
 *  Enforces label and domain length limits.
 */
static int
mowgli_dns_ns_name_pton(const char *src, unsigned char *dst, size_t dstsiz)
{
	unsigned char *label, *bp, *eom;
	char *cp;
	int c, n, escaped, e = 0;

	escaped = 0;
	bp = dst;
	eom = dst + dstsiz;
	label = bp++;

	while ((c = *src++) != 0)
	{
		if (escaped)
		{
			if (c == '[')	/* start a bit string label */
			{
				if ((cp = strchr(src, ']')) == NULL)
				{
					errno = EINVAL;	/* ??? */
					return -1;
				}

				if ((e = mowgli_dns_encode_bitsring(&src, cp + 2, &label, &bp, (const char *) eom)) != 0)
				{
					errno = e;
					return -1;
				}

				escaped = 0;
				label = bp++;

				if ((c = *src++) == 0)
				{
					goto done;
				}
				else if (c != '.')
				{
					errno = EINVAL;
					return -1;
				}

				continue;
			}
			else if ((cp = strchr(digits, c)) != NULL)
			{
				n = (cp - digits) * 100;

				if (((c = *src++) == 0) || ((cp = strchr(digits, c)) == NULL))
				{
					errno = EMSGSIZE;
					return -1;
				}

				n += (cp - digits) * 10;

				if (((c = *src++) == 0) || ((cp = strchr(digits, c)) == NULL))
				{
					errno = EMSGSIZE;
					return -1;
				}

				n += (cp - digits);

				if (n > 255)
				{
					errno = EMSGSIZE;
					return -1;
				}

				c = n;
			}

			escaped = 0;
		}
		else if (c == '\\')
		{
			escaped = 1;
			continue;
		}
		else if (c == '.')
		{
			c = (bp - label - 1);

			if ((c & MOWGLI_DNS_NS_CMPRSFLAGS) != 0)/* Label too big. */
			{
				errno = EMSGSIZE;
				return -1;
			}

			if (label >= eom)
			{
				errno = EMSGSIZE;
				return -1;
			}

			*label = c;

			/* Fully qualified ? */
			if (*src == '\0')
			{
				if (c != 0)
				{
					if (bp >= eom)
					{
						errno = EMSGSIZE;
						return -1;
					}

					*bp++ = '\0';
				}

				if ((bp - dst) > MOWGLI_DNS_NS_MAXCDNAME)
				{
					errno = EMSGSIZE;
					return -1;
				}

				return 1;
			}

			if ((c == 0) || (*src == '.'))
			{
				errno = EMSGSIZE;
				return -1;
			}

			label = bp++;
			continue;
		}

		if (bp >= eom)
		{
			errno = EMSGSIZE;
			return -1;
		}

		*bp++ = (unsigned char) c;
	}

	c = (bp - label - 1);

	if ((c & MOWGLI_DNS_NS_CMPRSFLAGS) != 0)/* Label too big. */
	{
		errno = EMSGSIZE;
		return -1;
	}

done:

	if (label >= eom)
	{
		errno = EMSGSIZE;
		return -1;
	}

	*label = c;

	if (c != 0)
	{
		if (bp >= eom)
		{
			errno = EMSGSIZE;
			return -1;
		}

		*bp++ = 0;
	}

	if ((bp - dst) > MOWGLI_DNS_NS_MAXCDNAME)	/* src too big */
	{
		errno = EMSGSIZE;
		return -1;
	}

	return 0;
}

/*
 * mowgli_dns_ns_name_pack(src, dst, dstsiz, dnptrs, lastdnptr)
 *  Pack domain name 'domain' into 'comp_dn'.
 * return:
 *  Size of the compressed name, or -1.
 * notes:
 *  'dnptrs' is an array of pointers to previous compressed names.
 *  dnptrs[0] is a pointer to the beginning of the message. The array
 *  ends with NULL.
 *  'lastdnptr' is a pointer to the end of the array pointed to
 *  by 'dnptrs'.
 * Side effects:
 *  The list of pointers in dnptrs is updated for labels inserted into
 *  the message as we compress the name.  If 'dnptr' is NULL, we don't
 *  try to compress names. If 'lastdnptr' is NULL, we don't update the
 *  list.
 */
static int
mowgli_dns_ns_name_pack(const unsigned char *src, unsigned char *dst, int dstsiz, unsigned char **dnptrs, unsigned char **lastdnptr)
{
	unsigned char *dstp;
	unsigned char **cpp, **lpp;
	const unsigned char *eob, *rmsg;
	const unsigned char *srcp;
	int n, l, first = 1;

	srcp = src;
	dstp = dst;
	eob = dstp + dstsiz;
	lpp = cpp = NULL;

	if (dnptrs != NULL)
	{
		if ((rmsg = *dnptrs++) != NULL)
		{
			for (cpp = dnptrs; *cpp != NULL; cpp++)
				(void) NULL;

			lpp = cpp;	/* end of list to search */
		}
	}
	else
	{
		rmsg = NULL;
	}

	/* make sure the domain we are about to add is legal */
	l = 0;

	do
	{
		int l0;

		n = *srcp;

		if ((n & MOWGLI_DNS_NS_CMPRSFLAGS) == MOWGLI_DNS_NS_CMPRSFLAGS)
		{
			errno = EMSGSIZE;
			return -1;
		}

		if ((l0 = labellen(srcp)) < 0)
		{
			errno = EINVAL;
			return -1;
		}

		l += l0 + 1;

		if (l > MOWGLI_DNS_NS_MAXCDNAME)
		{
			errno = EMSGSIZE;
			return -1;
		}

		srcp += l0 + 1;
	} while (n != 0);

	/* from here on we need to reset compression pointer array on error */
	srcp = src;

	do
	{
		/* Look to see if we can use pointers. */
		n = *srcp;

		if ((n != 0) && (rmsg != NULL))
		{
			l = mowgli_dns_dn_find(srcp, rmsg, (const unsigned char *const *) dnptrs,
					       (const unsigned char *const *) lpp);

			if (l >= 0)
			{
				if (dstp + 1 >= eob)
					goto cleanup;

				*dstp++ = (l >> 8) | MOWGLI_DNS_NS_CMPRSFLAGS;
				*dstp++ = l % 256;
				return dstp - dst;
			}

			/* Not found, save it. */
			if ((lastdnptr != NULL) && (cpp < lastdnptr - 1) && ((dstp - rmsg) < 0x4000) && first)
			{
				*cpp++ = dstp;
				*cpp = NULL;
				first = 0;
			}
		}

		/* copy label to buffer */
		if ((n & MOWGLI_DNS_NS_CMPRSFLAGS) == MOWGLI_DNS_NS_CMPRSFLAGS)
			/* Should not happen. */
			goto cleanup;

		n = labellen(srcp);

		if (dstp + 1 + n >= eob)
			goto cleanup;

		memcpy(dstp, srcp, n + 1);
		srcp += n + 1;
		dstp += n + 1;
	} while (n != 0);

	if (dstp > eob)
	{
cleanup:

		if (rmsg != NULL)
			*lpp = NULL;

		errno = EMSGSIZE;
		return -1;
	}

	return dstp - dst;
}

static int
mowgli_dns_ns_name_compress(const char *src, unsigned char *dst, size_t dstsiz, unsigned char **dnptrs, unsigned char **lastdnptr)
{
	unsigned char tmp[MOWGLI_DNS_NS_MAXCDNAME];

	if (mowgli_dns_ns_name_pton(src, tmp, sizeof tmp) == -1)
		return -1;

	return mowgli_dns_ns_name_pack(tmp, dst, dstsiz, dnptrs, lastdnptr);
}

static int
mowgli_dns_encode_bitsring(const char **bp, const char *end, unsigned char **labelp, unsigned char **dst, const char *eom)
{
	int afterslash = 0;
	const char *cp = *bp;
	char *tp, c;
	const char *beg_blen;
	char *end_blen = NULL;
	int value = 0, count = 0, tbcount = 0, blen = 0;

	beg_blen = end_blen = NULL;

	/* a bitstring must contain at least 2 characters */
	if (end - cp < 2)
		return EINVAL;

	/* XXX: currently, only hex strings are supported */
	if (*cp++ != 'x')
		return EINVAL;

	if (!isxdigit((*cp) & 0xff))	/* reject '\[x/BLEN]' */
		return EINVAL;

	for (tp = (char *) (dst + 1); cp < end && tp < eom; cp++)
	{
		switch ((c = *cp))
		{
		case ']':	/* end of the bitstring */

			if (afterslash)
			{
				if (beg_blen == NULL)
					return EINVAL;

				blen = (int) strtol(beg_blen, &end_blen, 10);

				if (*end_blen != ']')
					return EINVAL;
			}

			if (count)
				*tp++ = ((value << 4) & 0xff);

			cp++;	/* skip ']' */
			goto done;
		case '/':
			afterslash = 1;
			break;
		default:

			if (afterslash)
			{
				if (!isdigit(c & 0xff))
					return EINVAL;

				if (beg_blen == NULL)
				{
					if (c == '0')
						/* blen never begings with 0 */
						return EINVAL;

					beg_blen = cp;
				}
			}
			else
			{
				if (!isxdigit(c & 0xff))
					return EINVAL;

				value <<= 4;
				value += digitvalue[(int) c];
				count += 4;
				tbcount += 4;

				if (tbcount > 256)
					return EINVAL;

				if (count == 8)
				{
					*tp++ = value;
					count = 0;
				}
			}

			break;
		}
	}

done:

	if ((cp >= end) || (tp >= eom))
		return EMSGSIZE;

	/*
	 * bit length validation:
	 * If a <length> is present, the number of digits in the <bit-data>
	 * MUST be just sufficient to contain the number of bits specified
	 * by the <length>. If there are insignificant bits in a final
	 * hexadecimal or octal digit, they MUST be zero.
	 * RFC 2673, Section 3.2.
	 */
	if (blen > 0)
	{
		int traillen;

		if (((blen + 3) & ~3) != tbcount)
			return EINVAL;

		traillen = tbcount - blen;	/* between 0 and 3 */

		if (((value << (8 - traillen)) & 0xff) != 0)
			return EINVAL;
	}
	else
	{
		blen = tbcount;
	}

	if (blen == 256)
		blen = 0;

	/* encode the type and the significant bit fields */
	**labelp = MOWGLI_DNS_LABELTYPE_BITSTRING;
	**dst = blen;

	*bp = cp;
	*dst = (unsigned char *) tp;

	return 0;
}

/*
 * dn_find(domain, rmsg, dnptrs, lastdnptr)
 *  Search for the counted-label name in an array of compressed names.
 * return:
 *  offset from rmsg if found, or -1.
 * notes:
 *  dnptrs is the pointer to the first name on the list,
 *  not the pointer to the start of the message.
 */
static int
mowgli_dns_dn_find(const unsigned char *domain, const unsigned char *rmsg, const unsigned char *const *dnptrs, const unsigned char *const *lastdnptr)
{
	const unsigned char *dn, *cp, *sp;
	const unsigned char *const *cpp;
	unsigned int n;

	for (cpp = dnptrs; cpp < lastdnptr; cpp++)
	{
		sp = *cpp;

		/*
		 * terminate search on:
		 * root label
		 * compression pointer
		 * unusable offset
		 */
		while (*sp != 0 && (*sp & MOWGLI_DNS_NS_CMPRSFLAGS) == 0 && (sp - rmsg) < 0x4000)
		{
			dn = domain;
			cp = sp;

			while ((n = *cp++) != 0)
			{
				/*
				 * check for indirection
				 */
				switch (n & MOWGLI_DNS_NS_CMPRSFLAGS)
				{
				case 0:	/* normal case, n == len */
					n = labellen(cp - 1);	/* XXX */

					if (n != *dn++)
						goto next;

					for (; n > 0; n--)
						if (mowgli_dns_mklower(*dn++) != mowgli_dns_mklower(*cp++))
							goto next;

					/* Is next root for both ? */
					if ((*dn == '\0') && (*cp == '\0'))
						return sp - rmsg;

					if (*dn)
						continue;

					goto next;
				case MOWGLI_DNS_NS_CMPRSFLAGS:	/* indirection */
					cp = rmsg + (((n & 0x3f) << 8) | *cp);
					break;

				default:/* illegal type */
					errno = EMSGSIZE;
					return -1;
				}
			}

next:;
			sp += *sp + 1;
		}
	}

	errno = ENOENT;
	return -1;
}

/*
 *  Thinking in noninternationalized USASCII (per the DNS spec),
 *  convert this character to lower case if it's upper case.
 */
static int
mowgli_dns_mklower(int ch)
{
	if ((ch >= 0x41) && (ch <= 0x5A))
		return ch + 0x20;

	return ch;
}

/* From resolv/mkquery.c */

/*
 * Form all types of queries.
 * Returns the size of the result or -1.
 */
int
mowgli_dns_res_mkquery(const char *dname,	/* domain name */
		       int query_class, int type,	/* class and type of query */
		       unsigned char *buf,	/* buffer to put query */
		       int buflen)	/* size of buffer */
{
	mowgli_dns_resheader_t *hp;
	unsigned char *cp;
	int n;
	unsigned char *dnptrs[20], **dpp, **lastdnptr;

	/*
	 * Initialize header fields.
	 */
	if ((buf == NULL) || (buflen < MOWGLI_DNS_HFIXEDSIZE))
		return -1;

	memset(buf, 0, MOWGLI_DNS_HFIXEDSIZE);
	hp = (mowgli_dns_resheader_t *) buf;

	hp->id = 0;
	hp->opcode = MOWGLI_DNS_QUERY;
	hp->rd = 1;	/* recurse */
	hp->rcode = MOWGLI_DNS_NO_ERRORS;
	cp = buf + MOWGLI_DNS_HFIXEDSIZE;
	buflen -= MOWGLI_DNS_HFIXEDSIZE;
	dpp = dnptrs;
	*dpp++ = buf;
	*dpp++ = NULL;
	lastdnptr = dnptrs + sizeof dnptrs / sizeof dnptrs[0];

	if ((buflen -= MOWGLI_DNS_QFIXEDSIZE) < 0)
		return -1;

	if ((n = mowgli_dns_dn_comp(dname, cp, buflen, dnptrs, lastdnptr)) < 0)
		return -1;

	cp += n;
	buflen -= n;
	MOWGLI_DNS_NS_PUT16(type, cp);
	MOWGLI_DNS_NS_PUT16(query_class, cp);
	hp->qdcount = htons(1);

	return cp - buf;
}
