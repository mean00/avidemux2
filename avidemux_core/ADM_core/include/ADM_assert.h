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
#include <pthread.h>
#include <assert.h>
#include "ADM_inttype.h"

#if defined(__MINGW32__)
	#include <_mingw.h>

	#if defined(__MINGW64_VERSION_STR)
		#if defined (__WIN64)
			#include <intrin.h>
		#endif

		#include <wchar.h>
	#endif
#endif


#define ADM_assert(x) { if(!(x)) {ADM_backTrack("Assert failed :"#x,__LINE__,__FILE__);  }}

/* Functions we want to override to have better os support / debug / error control */

#ifdef __cplusplus
extern "C" {
#endif
/* Our crash  / assert functions */
typedef void ADM_saveFunction(void);
typedef void ADM_fatalFunction(const char *title, const char *info);

void            ADM_setCrashHook(ADM_saveFunction *save, ADM_fatalFunction *fatal);
void            ADM_backTrack(const char *info,int lineno,const char *file);
/* Our crash  / assert functions */

/* Replacement for fread & friends */
size_t          ADM_fread (void *ptr, size_t size, size_t n, FILE *sstream);
size_t          ADM_fwrite (const void *ptr, size_t size, size_t n, FILE *sstream);
FILE            *ADM_fopen (const char *file, const char *mode);
int             ADM_fclose (FILE *file);
uint8_t         ADM_fileExist(const char *name);
uint8_t         ADM_mkdir(const char *name);

/* Replacements for memory allocation functions */
extern void     *ADM_alloc(size_t size);
extern void     *ADM_calloc(size_t nbElm,size_t elSize);
extern void     *ADM_realloc(void *in,size_t size);
extern void     ADM_dezalloc(void *ptr);
extern char     *ADM_strdup( const char *in);
/* Endianness stuff */
uint64_t 	ADM_swap64(uint64_t in);
uint32_t 	ADM_swap32(uint32_t in);
uint16_t 	ADM_swap16(uint16_t in);
//static inline uint32_t dontswap(uint32_t in) {return in;};

/* */
void            ADM_usleep(unsigned long us);


typedef void *(* adm_fast_memcpy)(void *to, const void *from, size_t len);
extern adm_fast_memcpy myAdmMemcpy;

#define ADM_memalign(x,y) ADM_alloc(y)

#define ADM_dealloc(x) ADM_dezalloc( (void *)x)
#define memcpy myAdmMemcpy

// Override fread/fwrite ..
#define fread   ADM_fread
#define fwrite  ADM_fwrite
#define fopen   ADM_fopen
#define fclose  ADM_fclose

#if !defined(__APPLE__) && !defined(__WIN64)
#ifndef ADM_LEGACY_PROGGY
  #define malloc #error
  #define realloc #error
  #define memalign #error
  #define free  #error
  #undef strdup
  #define strdup #error
  #define calloc #error
#else
  #define malloc ADM_alloc
  #define realloc ADM_realloc
  #define memalign(x,y) ADM_alloc(y)
  #define free  ADM_dezalloc
  #undef strdup
  #define strdup ADM_strdup
  #define calloc ADM_calloc
#endif
#endif    // __APPLE__
#ifdef __cplusplus
}
#endif
// Compatibility with fprintf etc.. with long long & win32
// ADM_cleanupPath returns a cleaned up copy of the parameter
#ifdef __WIN32
        #define LLX "I64x"
        #define LLU "I64u"
        #define LLD "I64d"
        #define LU  "lu"
        #define LD  "ld"
        #define LX  "lx"

		char *ADM_slashToBackSlash(const char *in);
        #define ADM_cleanupPath(x) ADM_slashToBackSlash(x)
#else
    
        #define LLX PRIx64
        #define LLU PRIu64
        #define LLD PRId64
        #define LX  PRIx32
        #define LD  PRIi32
        #define LU  PRIu32

    
        #define ADM_cleanupPath(x) ADM_strdup(x)
#endif


#endif
// EOF
