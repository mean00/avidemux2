/** *************************************************************************
    \fn ADM_inttype.h
    \brief Defint plaform agnostic var type
                      
    copyright            : (C) 2008 by mean
    
 ***************************************************************************/

#ifndef ADM_INTTYPE_H
#define ADM_INTTYPE_H

#include "ADM_coreConfig.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
   	The maximum width/height is MAXIMUM_SIZE 768*768 for now
*/
#define MAXIMUM_SIZE 2048

typedef unsigned char ADM_filename;

#ifdef HAVE_STDINT_H
#define __STDC_CONSTANT_MACROS
#define __STDC_LIMIT_MACROS
#include <stdint.h>
#define GOT_TYPES
#endif
#ifdef NEED_STDINT_GCC
#include <stdint-gcc.h>
#endif //NEED_STDINT_GCC
#ifdef HAVE_INTTYPES_H
	#define __STDC_FORMAT_MACROS
	#include <inttypes.h>
	#define GOT_TYPES
#ifndef SCNu8
#define SCNu8 "u"
#endif
#endif

#ifndef GOT_TYPES
#ifdef ADM_CPU_64BIT
#define int32_t 	signed int
#define int64_t 	signed long int 
#define uint64_t 	unsigned long  int
#else
#define int32_t 	signed long  int
#define int64_t 	signed long  long
#define uint64_t 	unsigned long  long
#endif	// ADM_CPU_64BIT

#define uint8_t  	unsigned char
#define int8_t   	signed char
#define int16_t 	signed short int
#define uint16_t 	unsigned short int
#define uint32_t 	unsigned long  int
#endif	// GOT_TYPES

#define UNUSED_ARG(a) (void)a


#define MKFCC(a,b,c,d)   ((d<<24)+(c<<16)+(b<<8)+a)
#define MKFCCR(a,b,c,d)   ((a<<24)+(b<<16)+(c<<8)+d)

// 0 means error, 1 means ok, 2 means ignore
// 2 is seldom used

#define ADM_ERR 0
#define ADM_OK 	1
#define ADM_IGN 2

#endif
