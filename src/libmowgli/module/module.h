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

#ifndef __MOWGLI_MODULE_H__
#define __MOWGLI_MODULE_H__

typedef void *mowgli_module_t;

extern mowgli_module_t mowgli_module_open(const char *path);
extern void *mowgli_module_symbol(mowgli_module_t module, const char *symbol);
extern void mowgli_module_close(mowgli_module_t module);

typedef void mowgli_interface_t;

typedef struct {
	const char *id;
	uint32_t abirev;
} mowgli_interface_base_t;

extern void mowgli_interface_register(mowgli_interface_t *iface);
extern void mowgli_interface_unregister(mowgli_interface_t *iface);
extern mowgli_interface_t *mowgli_interface_get(const char *id, uint32_t abirev);

#endif
