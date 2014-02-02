/*
 * libmowgli: A collection of useful routines for programming.
 * module.h: Loadable modules.
 *
 * Copyright (c) 2007, 2014 William Pitcock <william@dereferenced.org>
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

static mowgli_patricia_t *mowgli_interface_dict = NULL;
static mowgli_mutex_t mowgli_interface_lock;

void
mowgli_interface_bootstrap(void)
{
	mowgli_interface_dict = mowgli_patricia_create(NULL);
	mowgli_mutex_init(&mowgli_interface_lock);
}

void
mowgli_interface_register(mowgli_interface_t *iface)
{
	mowgli_interface_base_t *base_iface = iface;

	mowgli_mutex_lock(&mowgli_interface_lock);
	mowgli_patricia_add(mowgli_interface_dict, base_iface->id, iface);
	mowgli_mutex_unlock(&mowgli_interface_lock);
}

void
mowgli_interface_unregister(mowgli_interface_t *iface)
{
	mowgli_interface_base_t *base_iface = iface;

	mowgli_mutex_lock(&mowgli_interface_lock);
	mowgli_patricia_delete(mowgli_interface_dict, base_iface->id);
	mowgli_mutex_unlock(&mowgli_interface_lock);
}

mowgli_interface_t *
mowgli_interface_get(const char *id, uint32_t abirev)
{
	mowgli_interface_base_t *base_iface;

	mowgli_mutex_lock(&mowgli_interface_lock);

	base_iface = mowgli_patricia_retrieve(mowgli_interface_dict, id);
	if (base_iface->abirev != abirev)
	{
		mowgli_log("requested interface %s, abi mismatch %d != %d", id, abirev, base_iface->abirev);
		base_iface = NULL;
	}

	mowgli_mutex_unlock(&mowgli_interface_lock);

	return base_iface;
}
