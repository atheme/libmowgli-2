/*
 * libmowgli: A collection of useful routines for programming.
 * mowgli_string.c: Immutable string buffers with cheap manipulation.
 *
 * Copyright (c) 2007 William Pitcock <nenolod -at- sacredspiral.co.uk>
 * Copyright (c) 2007 Pippijn van Steenhoven <pippijn -at- one09.net>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice is present in all copies.
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

mowgli_string_t *
mowgli_string_create(void)
{
	mowgli_string_t *self = mowgli_alloc(sizeof(mowgli_string_t));

	self->size = 64;
	self->pos = 0;
	self->str = mowgli_alloc(self->size);

	self->append = &mowgli_string_append;
	self->append_char = &mowgli_string_append_char;
	self->reset = &mowgli_string_reset;
	self->destroy = &mowgli_string_destroy;

	return self;
}

void
mowgli_string_reset(mowgli_string_t *self)
{
	return_if_fail(self != NULL);

	self->str[0] = self->pos = 0;
}

void
mowgli_string_destroy(mowgli_string_t *self)
{
	return_if_fail(self != NULL);

	mowgli_free(self->str);
	mowgli_free(self);
}

void
mowgli_string_append(mowgli_string_t *self, const char *src, size_t n)
{
	if (self->size - self->pos <= n)
	{
		char *new_ptr;

		self->size = MAX(self->size * 2, self->pos + n + 8);

		new_ptr = mowgli_alloc(self->size);
		mowgli_strlcpy(new_ptr, self->str, self->size);

		mowgli_free(self->str);
		self->str = new_ptr;
	}

	memcpy(self->str + self->pos, src, n);
	self->pos += n;
	self->str[self->pos] = 0;
}

void
mowgli_string_append_char(mowgli_string_t *self, const char c)
{
	if (self->size - self->pos <= 1)
	{
		char *new_ptr;

		self->size = MAX(self->size * 2, self->pos + 9);

		new_ptr = mowgli_alloc(self->size);
		mowgli_strlcpy(new_ptr, self->str, self->size);

		mowgli_free(self->str);
		self->str = new_ptr;
	}

	self->str[self->pos++] = c;
	self->str[self->pos] = 0;
}

/*
 * Copyright (c) 1998 Todd C. Miller <Todd.Miller@courtesan.com>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

size_t
mowgli_strlcat(char *dst, const char *src, size_t siz)
{
	char *d = dst;
	const char *s = src;
	size_t n = siz;
	size_t dlen;

	/* Find the end of dst and adjust bytes left but don't go past end */
	while (n-- != 0 && *d != '\0')
	{
		d++;
	}

	dlen = d - dst;
	n = siz - dlen;

	if (n == 0)
		return dlen + strlen(s);

	while (*s != '\0')
	{
		if (n != 1)
		{
			*d++ = *s;
			n--;
		}

		s++;
	}

	*d = '\0';

	return dlen + (s - src);/* count does not include NUL */
}

size_t
mowgli_strlcpy(char *dst, const char *src, size_t siz)
{
	char *d = dst;
	const char *s = src;
	size_t n = siz;

	/* Copy as many bytes as will fit */
	if (n != 0)
		while (--n != 0)
		{
			if ((*d++ = *s++) == '\0')
				break;
		}

	/* Not enough room in dst, add NUL and traverse rest of src */

	if (n == 0)
	{
		if (siz != 0)
			*d = '\0';		/* NUL-terminate dst */

		while (*s++)
		{ }
	}

	return s - src - 1;	/* count does not include NUL */
}

ssize_t
mowgli_writef(mowgli_descriptor_t fd, const char *fmt, ...)
{
	size_t len;
	va_list va;
	char buf[16384];

	return_val_if_fail(fmt != NULL, -1);

	va_start(va, fmt);
	len = vsnprintf(buf, sizeof buf, fmt, va);
	va_end(va);

#ifdef _WIN32
	return send(fd, buf, len, 0);
#else
	return write(fd, buf, len);
#endif
}
