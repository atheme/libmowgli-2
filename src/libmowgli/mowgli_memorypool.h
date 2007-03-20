/*  Audacious
 *  Copyright (c) 2007 William Pitcock <nenolod -at- atheme.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; under version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef __MOWGLI_MEMORYPOOL_H__
#define __MOWGLI_MEMORYPOOL_H__

typedef struct mowgli_memorypool_t_ mowgli_memorypool_t;

mowgli_memorypool_t * mowgli_memory_pool_new(void);
mowgli_memorypool_t * mowgli_memory_pool_with_custom_destructor(mowgli_destructor_t destructor);

void * mowgli_memory_pool_add(mowgli_memorypool_t * pool, void * ptr);
void * mowgli_memory_pool_allocate(mowgli_memorypool_t * pool, size_t sz);
void mowgli_memory_pool_release(mowgli_memorypool_t * pool, void * addr);

void mowgli_memory_pool_cleanup(mowgli_memorypool_t * pool);

void mowgli_memory_pool_destroy(mowgli_memorypool_t * pool);

char * mowgli_memory_pool_strdup(mowgli_memorypool_t * pool, char * src);

#define mowgli_memory_pool_alloc_object(pool, obj) \
	mowgli_memory_pool_allocate(pool, sizeof(obj))

#endif
