/*
 * default memory allocator for libavutil
 * Copyright (c) 2002 Fabrice Bellard
 *
 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/**
 * @file
 * default memory allocator for libavutil
 */

#include "config.h"

#include <limits.h>
#include <stdlib.h>
#include <string.h>
#if HAVE_MALLOC_H
#include <malloc.h>
#endif

#include "avutil.h"
#include "mem.h"

/* here we can use OS-dependent allocation functions */
#undef free
#undef malloc
#undef realloc

#ifdef MALLOC_PREFIX

#define malloc         AV_JOIN(MALLOC_PREFIX, malloc)
#define memalign       AV_JOIN(MALLOC_PREFIX, memalign)
#define posix_memalign AV_JOIN(MALLOC_PREFIX, posix_memalign)
#define realloc        AV_JOIN(MALLOC_PREFIX, realloc)
#define free           AV_JOIN(MALLOC_PREFIX, free)

void *malloc(size_t size);
void *memalign(size_t align, size_t size);
int   posix_memalign(void **ptr, size_t align, size_t size);
void *realloc(void *ptr, size_t size);
void  free(void *ptr);

#endif /* MALLOC_PREFIX */

/* You can redefine av_malloc and av_free in your project to use your
   memory allocator. You do not need to suppress this file because the
   linker will do it automatically. */

void *av_malloc(FF_INTERNAL_MEM_TYPE size)
{
#ifdef __APPLE__
	return malloc(size);
#else
	char *c;

	uint64_t l, lorg;
	uint32_t *backdoor;

	l = (uint64_t)malloc(size + 32);

	// Get next boundary
	lorg = l;
	l = (l + 15) & 0xfffffffffffffff0LL;
	l += 16;
	c = (char*)l;
	backdoor = (uint32_t*)(c - 8);
	*backdoor = (0xdead << 16) + l - lorg;
	backdoor[1] = size;

	return c;
 #endif

}

void *av_realloc(void *ptr, FF_INTERNAL_MEM_TYPE newsize)
{
#ifdef __APPLE__
	if (!ptr)
		return av_malloc(newsize);
	if (!newsize)
	{
		av_free(ptr);
		return NULL;
	}
#else
	void *nalloc;

	if (!ptr)
		return av_malloc(newsize);

	if (!newsize) 
	{
		av_free(ptr);
		return NULL;
	}

	// now we either shrink them or expand them
	// in case of shrink, we do nothing
	// in case of expand we have to copy
	// Do copy everytime (slower)
	uint32_t *backdoor;
	uint32_t size, offset;
	char *c = (char*)ptr;

	backdoor = (uint32_t*)ptr;
	backdoor -= 2;

	assert(((*backdoor) >> 16) == 0xdead);

	offset = backdoor[0] & 0xffff;
	size = backdoor[1];

	if(size >= newsize) // do nothing
		return ptr;

	// Allocate a new one
	nalloc = av_malloc(newsize);
	memcpy(nalloc, ptr, size);
	av_free(ptr);

	return nalloc;
 #endif


}

void av_free(void *ptr)
{
#ifdef __APPLE__
	if (!ptr)
		return;

	free(ptr);
 #else
	uint32_t *backdoor;
	uint32_t size, offset;
	char *c = (char*)ptr;

	if (!ptr)
		return;

	backdoor = (uint32_t*)ptr;
	backdoor -= 2;

	if (*backdoor == 0xbeefbeef)
		assert(0);

	assert(((*backdoor) >> 16) == 0xdead);

	offset = backdoor[0] & 0xffff;
	size = backdoor[1];
	*backdoor = 0xbeefbeef; // Scratch sig

	free(c - offset);
 #endif
	
}

void av_freep(void *arg)
{
    void **ptr= (void**)arg;
    av_free(*ptr);
    *ptr = NULL;
}

void *av_mallocz(FF_INTERNAL_MEM_TYPE size)
{
    void *ptr = av_malloc(size);
    if (ptr)
        memset(ptr, 0, size);
    return ptr;
}

char *av_strdup(const char *s)
{
    char *ptr= NULL;
    if(s){
        int len = strlen(s) + 1;
        ptr = av_malloc(len);
        if (ptr)
            memcpy(ptr, s, len);
    }
    return ptr;
}

