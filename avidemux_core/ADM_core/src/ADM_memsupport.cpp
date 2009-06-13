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
#include "ADM_default.h"
#include "ADM_threads.h"

#undef memalign
#undef malloc
#undef free
#undef realloc

static uint32_t ADM_consumed = 0;
static admMutex memAccess("MemAccess");
static int doMemStat = 0;

#if defined(ADM_DEBUG) && defined(FIND_LEAKS)
#define _DEBUG_NEW_CALLER_ADDRESS __builtin_return_address(0)
extern void* operator new(size_t size, const char* file, int line);
extern void operator delete(void* pointer, const char* file, int line) throw();
extern size_t getSizeFromPointer(void* ptr);
#endif

extern "C"
{
	void *av_malloc(unsigned int size);
	void av_free(void *ptr);
	void *av_realloc(void *ptr, unsigned int size);
}

void ADM_memStat(void);
void ADM_memStatInit(void);
void ADM_memStatEnd(void);

void ADM_memStatInit(void)
{
	ADM_consumed = 0;
	doMemStat = 1;
}

void ADM_memStatEnd(void)
{
	doMemStat = 0;
}

void ADM_memStat(void)
{
	printf("Global mem stat\n______________\n");
	printf("\tMemory consumed: %"LU" (MB)\n", ADM_consumed >> 20);
}

#if !defined(ADM_DEBUG) || !defined(FIND_LEAKS)
/**
    \fn ADM_calloc(size_t nbElm,size_t elSize);
    \brief Replacement for system Calloc using our memory management
    \param nbElem : # of elements to allocate
    \param elSize : Size of one element in bytes
    \return pointer
*/
void *ADM_calloc(size_t nbElm, size_t elSize)
{
	void *out = ADM_alloc(nbElm * elSize);
	memset(out, 0, nbElm * elSize);
	return out;
}

void *ADM_alloc(size_t size)
{
#ifdef __APPLE__
	return malloc(size);
#else
	char *c;

	uint64_t l, lorg;
	uint32_t *backdoor;
	int dome = doMemStat;

	if(dome)
		memAccess.lock();

	l = (uint64_t)malloc(size + 32);

	// Get next boundary
	lorg = l;
	l = (l + 15) & 0xfffffffffffffff0LL;
	l += 16;
	c = (char*)l;
	backdoor = (uint32_t*)(c - 8);
	*backdoor = (0xdead << 16) + l - lorg;
	backdoor[1] = size;

	if(dome)
		memAccess.unlock();

	ADM_consumed += size;

	return c;
#endif
}

void ADM_dezalloc(void *ptr)
{
#ifdef __APPLE__
	if (!ptr)
		return;

	free(ptr);
#else
	int dome = doMemStat;
	uint32_t *backdoor;
	uint32_t size, offset;
	char *c = (char*)ptr;

	if (!ptr)
		return;

	backdoor = (uint32_t*)ptr;
	backdoor -= 2;

	if (*backdoor == 0xbeefbeef)
	{
		printf("Double free gotcha!\n");
		ADM_assert(0);
	}

	ADM_assert(((*backdoor) >> 16) == 0xdead);

	offset = backdoor[0] & 0xffff;
	size = backdoor[1];
	*backdoor = 0xbeefbeef; // Scratch sig

	if (dome)
		memAccess.lock();

	free(c - offset);
	ADM_consumed -= size;

	if(dome)
		memAccess.unlock();
#endif
}

void *operator new( size_t t)
{
	return ADM_alloc(t);
}

void *operator new[] ( size_t t)
{
	return ADM_alloc(t);
}

void operator delete (void *c)
{
	ADM_dezalloc(c);
}

void operator delete[] (void *c)
{
	ADM_dezalloc(c);
}
//********************************
// lavcodec wrapper
//********************************
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

/**
 * av_realloc semantics (same as glibc): if ptr is NULL and size > 0,
 * identical to malloc(size). If size is zero, it is identical to
 * free(ptr) and NULL is returned.
 */
void *ADM_realloc(void *ptr, size_t newsize)
{
#ifdef __APPLE__
	if(!ptr)
		return ADM_alloc(newsize);

	if (!newsize)
	{
		ADM_dealloc(ptr);
		return NULL;
	}

	return realloc(ptr, newsize);
#else
	void *nalloc;

	if(!ptr)
		return ADM_alloc(newsize);

	if(!newsize)
	{
		ADM_dealloc(ptr);
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

	ADM_assert(((*backdoor) >> 16) == 0xdead);

	offset = backdoor[0] & 0xffff;
	size = backdoor[1];

	if(size >= newsize) // do nothing
		return ptr;

	// Allocate a new one
	nalloc = ADM_alloc(newsize);
	memcpy(nalloc, ptr, size);
	ADM_dealloc(ptr);

	return nalloc;
#endif
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

#else

void *ADM_alloc(size_t size)
{
	return operator new(size, (char*)_DEBUG_NEW_CALLER_ADDRESS, 0);
}

void *ADM_calloc(size_t nbElm,size_t elSize)
{
	void *out = operator new(nbElm*elSize, (char*)_DEBUG_NEW_CALLER_ADDRESS, 0);

	memset(out,0,nbElm*elSize);
	return out;
}

char *ADM_strdup(const char *in)
{
    if(!in)
		return NULL;

	int size = strlen(in) + 1;
	char *out = (char *)(operator new(size, (char*)_DEBUG_NEW_CALLER_ADDRESS, 0));

	memcpy(out, in, size);

	return out;
}

void ADM_dezalloc(void *ptr)
{
	operator delete(ptr, (char*)_DEBUG_NEW_CALLER_ADDRESS, 0);
}

void *ADM_realloc(void *ptr, size_t newsize)
{
	void *nalloc;

    if(!ptr)
		return operator new(newsize, (char*)_DEBUG_NEW_CALLER_ADDRESS, 0);

    if(!newsize)
    {
		operator delete(ptr, (char*)_DEBUG_NEW_CALLER_ADDRESS, 0);
		return NULL;
    }

	uint32_t size = getSizeFromPointer(ptr);

	if (size >= newsize)
		return ptr;

	nalloc = operator new(newsize, (char*)_DEBUG_NEW_CALLER_ADDRESS, 0);
	memcpy(nalloc,ptr,getSizeFromPointer(ptr));
	operator delete(ptr, (char*)_DEBUG_NEW_CALLER_ADDRESS, 0);

	return nalloc;
}

extern "C"
{
	void *av_malloc(unsigned int size)
	{
		return operator new(size, (char*)_DEBUG_NEW_CALLER_ADDRESS, 0);
	}
	void av_freep(void *arg)
	{
		void **ptr = (void**)arg;
		av_free(*ptr);
		*ptr = NULL;
	}

	void *av_mallocz(unsigned int size)
	{
		void *ptr;

		ptr = operator new(size, (char*)_DEBUG_NEW_CALLER_ADDRESS, 0);

		if (ptr)
			memset(ptr, 0, size);

		return ptr;
	}
}

char *av_strdup(const char *s)
{
    char *ptr;
    int len;
    len = strlen(s) + 1;
	ptr = (char *)operator new(len, (char*)_DEBUG_NEW_CALLER_ADDRESS, 0);

    if (ptr)
        memcpy(ptr, s, len);

    return ptr;
}

void *av_realloc(void *ptr, unsigned int newsize)
{
	if(!ptr)
		return operator new(newsize, (char*)_DEBUG_NEW_CALLER_ADDRESS, 0);
	else
		return ADM_realloc(ptr,newsize);
}

void av_free(void *ptr)
{
	if(ptr)
		operator delete(ptr, (char*)_DEBUG_NEW_CALLER_ADDRESS, 0);
}
#endif
// EOF
