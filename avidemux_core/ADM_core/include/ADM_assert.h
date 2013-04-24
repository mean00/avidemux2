/** *************************************************************************
    \fn ADM_assert.h
    \brief Replacement for assert etc ... (low level funcs)

    copyright            : (C) 2008 by mean

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#ifndef ADM_ASSERT_H
#define ADM_ASSERT_H

#include "ADM_core6_export.h"
#include <assert.h>

#ifdef _WIN32
	//#include <intrin.h>
	#include <wchar.h>
#endif

#include "ADM_inttype.h"
#include "ADM_crashdump.h"

#define ADM_assert(x) { if(!(x)) {ADM_backTrack("Assert failed :"#x,__LINE__,__FILE__);  }}

/* Functions we want to override to have better os support / debug / error control */

#ifdef __cplusplus
#include <new>
extern "C" {
#endif

/* Replacement for fread & friends */
ADM_CORE6_EXPORT size_t          ADM_fread (void *ptr, size_t size, size_t n, FILE *sstream);
ADM_CORE6_EXPORT size_t          ADM_fwrite (const void *ptr, size_t size, size_t n, FILE *sstream);
ADM_CORE6_EXPORT FILE            *ADM_fopen (const char *file, const char *mode);
ADM_CORE6_EXPORT int             ADM_fclose (FILE *file);
ADM_CORE6_EXPORT uint8_t         ADM_fileExist(const char *name);
ADM_CORE6_EXPORT uint8_t         ADM_mkdir(const char *name);
ADM_CORE6_EXPORT uint8_t         ADM_eraseFile(const char *name);
ADM_CORE6_EXPORT int64_t         ADM_fileSize(const char *file);
/* Replacements for memory allocation functions */
ADM_CORE6_EXPORT void     *ADM_alloc(size_t size);
ADM_CORE6_EXPORT void     *ADM_memalign(size_t align,size_t size);
ADM_CORE6_EXPORT void     *ADM_calloc(size_t nbElm,size_t elSize);
ADM_CORE6_EXPORT void     *ADM_realloc(void *in,size_t size);
ADM_CORE6_EXPORT void     ADM_dezalloc(void *ptr);
ADM_CORE6_EXPORT char     *ADM_strdup( const char *in);
/* Endianness stuff */
uint64_t 	ADM_swap64(uint64_t in);
ADM_CORE6_EXPORT uint32_t 	ADM_swap32(uint32_t in);
ADM_CORE6_EXPORT uint16_t 	ADM_swap16(uint16_t in);
//static inline uint32_t dontswap(uint32_t in) {return in;};

/* */
ADM_CORE6_EXPORT void            ADM_usleep(unsigned long us);

#ifndef __APPLE__
  typedef void *(* adm_fast_memcpy)(void *to, const void *from, size_t len);
  extern ADM_CORE6_EXPORT adm_fast_memcpy myAdmMemcpy;
#endif


#define ADM_dealloc(x) ADM_dezalloc( (void *)x)

#ifndef __APPLE__
  #define memcpy myAdmMemcpy
#endif

// Override fread/fwrite ..
#define fread   ADM_fread
#define fwrite  ADM_fwrite
#define fopen   ADM_fopen
#define fclose  ADM_fclose

#if defined(__APPLE__) || defined(_WIN64) || defined(__HAIKU__)
        #define NO_ADM_MEMCHECK
#endif

  
#ifdef __cplusplus
}
  
#endif

// ADM_cleanupPath returns a cleaned up copy of the parameter
#ifdef _WIN32
	ADM_CORE6_EXPORT char *ADM_slashToBackSlash(const char *in);
	#define ADM_cleanupPath(x) ADM_slashToBackSlash(x)
#else
	#define ADM_cleanupPath(x) ADM_strdup(x)
#endif
#endif
// EOF
