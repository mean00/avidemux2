/** *************************************************************************
    \fn ADM_cpuCap.h
    \brief Handle cpu capabilities (MMX/SSE/...)
                      
    copyright            : (C) 2008 by mean
    
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#ifndef ADM_CPUCAP_H
#define ADM_CPUCAP_H

#include "ADM_core6_export.h"
#include "ADM_coreConfig.h"

#ifdef ADM_CPU_X86

#define CHECK_Z(x) {if(CpuCaps::myCpuCaps & ADM_CPUCAP_##x & CpuCaps::myCpuMask) return 1; else return 0;} 

#else
#define         CHECK_Z(x) {return 0;}
#endif

typedef enum 
{
        ADM_CPUCAP_NONE   =1,
        ADM_CPUCAP_MMX    =1<<1,
        ADM_CPUCAP_MMXEXT =1<<2,
        ADM_CPUCAP_3DNOW  =1<<3,
        ADM_CPUCAP_3DNOWEXT  =1<<4,
        ADM_CPUCAP_SSE    =1<<5,
        ADM_CPUCAP_SSE2   =1<<6,
        ADM_CPUCAP_SSE3   =1<<7,
        ADM_CPUCAP_SSSE3  =1<<8,
        ADM_CPUCAP_ALTIVEC=1<<9,
        
        ADM_CPUCAP_ALL=0x0fffffff
} ADM_CPUCAP;
/**
    \class CpuCaps
    \brief Helper class to get CPU capabilities (MMX/SSE/...)
*/
class ADM_CORE6_EXPORT CpuCaps
{
protected:
        static uint32_t      myCpuCaps;
        static uint32_t      myCpuMask;
public:
	static void 	init( void);
        static bool     setMask(uint32_t mask);
        static uint32_t getMask();
	static uint8_t 	hasMMX (void) {CHECK_Z(MMX)};
	static uint8_t 	hasMMXEXT (void){CHECK_Z(MMXEXT)};
	static uint8_t 	has3DNOW (void) {CHECK_Z(3DNOW)};
	static uint8_t 	hasSSE (void) {CHECK_Z(SSE)};
	static uint8_t 	hasSSE2 (void){CHECK_Z(SSE2)};
	static uint8_t 	hasSSE3 (void){CHECK_Z(SSE3)};
	static uint8_t 	hasSSSE3 (void){CHECK_Z(SSSE3)};
	static uint8_t 	has3DNOWEXT(void){CHECK_Z(3DNOWEXT)};


};
ADM_CORE6_EXPORT int ADM_cpu_num_processors(void); // Returns the # of cores/CPUs
#endif
