//
// C++ Interface: ADM_memsupport
//
// Description:
//	Wrapper for all memory alloc/dealloc
//
//	Ensures 16 byte alignment for all memory allocations on Linux and Windows (to
//  support PPC and SSE).
//
//  Mac OS X is exempt as it automatically ensures 16 byte alignment and overriding
//  the new/delete operators with custom memory management clashes with Qt4.
//
// Author: mean <fixounet@free.fr>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <malloc.h>
#include "ADM_default.h"
#include "ADM_threads.h"
#include "ADM_memsupport.h"


#undef memalign
#undef malloc
#undef free
#undef realloc

static uint32_t ADM_consumed = 0;
static uint32_t ADM_maxConsumed = 0;
static admMutex memAccess("MemAccess");
static int doMemStat = 0;

static void *ADM_aligned_alloc(size_t size);
static void ADM_aligned_free(void *ptr);
static void *ADM_aligned_realloc(void *ptr,size_t size);
static void *ADM_aligned_memalign(size_t align,size_t size);


extern "C"
{
	void *av_malloc(unsigned int size);
	void av_free(void *ptr);
	void *av_realloc(void *ptr, unsigned int size);
}


/**
    \fn ADM_calloc(size_t nbElm,size_t elSize);
    \brief Replacement for system Calloc using our memory management
    \param nbElem : # of elements to allocate
    \param elSize : Size of one element in bytes
    \return pointer
*/
void *ADM_calloc(size_t nbElm, size_t elSize)
{
	void *out = ADM_aligned_alloc(nbElm * elSize);
	memset(out, 0, nbElm * elSize);
	return out;
}

void *ADM_alloc(size_t size)
{
	return ADM_aligned_alloc(size);
}
void ADM_dezalloc(void *ptr)
{
	if (!ptr)
		return;

	ADM_aligned_free(ptr);
}
void *ADM_realloc(void *ptr, size_t newsize)
{
	if(!ptr)
		return ADM_alloc(newsize);

	if (!newsize)
	{
		ADM_dealloc(ptr);
		return NULL;
	}

	return ADM_aligned_realloc(ptr, newsize);
}

#ifdef __MINGW32__
void *ADM_aligned_alloc(size_t size)
{
    return _aligned_malloc(size,16);
}
void ADM_aligned_free(void *ptr)
{
    return _aligned_free(ptr);
}
void *ADM_aligned_realloc(void *ptr,size_t size)
{
    return _aligned_realloc(ptr,size,16);
}
void *ADM_aligned_memalign(size_t align,size_t size)
{
    ADM_assert(align<=16);
    return ADM_aligned_alloc(size);
}
#else
void *ADM_aligned_alloc(size_t size)
{
    return memalign(16,size);
}
void ADM_aligned_free(void *ptr)
{
    return free(ptr);
}
void *ADM_aligned_realloc(void *ptr,size_t size)
{
    return realloc(ptr,size);
}
void *ADM_aligned_memalign(size_t align,size_t size)
{
    ADM_assert(align<=16);
    return ADM_aligned_alloc(size);
}
#endif
//********************************
// lavcodec wrapper
//********************************
#if defined(WRAP_LAV_ALLOC)
extern "C"
{
	void *av_malloc(unsigned int size)
	{
 		return ADM_alloc(size);
	}

	void av_freep(void *arg)
	{
		void **ptr= (void**)arg;
		av_free(*ptr);
		*ptr = NULL;
	}

	void *av_mallocz(unsigned int size)
	{
		void *ptr;

		ptr = av_malloc(size);

		if (ptr)
			memset(ptr, 0, size);

		return ptr;
	}

	char *av_strdup(const char *s)
	{
		char *ptr;
		int len;

		len = strlen(s) + 1;
		ptr = (char *)av_malloc(len);

		if (ptr)
			memcpy(ptr, s, len);

		return ptr;
	}
}
void *av_realloc(void *ptr, unsigned int newsize)
{
	return ADM_realloc(ptr,newsize);
}

/* NOTE: ptr = NULL is explicetly allowed */
void av_free(void *ptr)
{
	if(ptr)
		ADM_dealloc(ptr);
}
#endif
char *ADM_strdup(const char *in)
{
	if(!in)
		return NULL;

	uint32_t l = strlen(in);
	char *out;

	out = (char*)ADM_alloc(l + 1);
	memcpy(out, in, l+1);

	return out;
}


// EOF
