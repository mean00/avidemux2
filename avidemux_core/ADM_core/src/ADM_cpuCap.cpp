//
// C++ Implementation: ADM_cpuCap
//
// Description:
//
//
// Author: mean <fixounet@free.fr>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "ADM_coreConfig.h"
#include "ADM_default.h"

#if defined(_WIN32)
#include <pthread.h>
#elif defined(__APPLE__) || defined(ADM_BSD_FAMILY) && !defined(__HAIKU__)
#include <sys/types.h>
#include <sys/sysctl.h>
#else
#include <string.h>
#include <sched.h>
#endif

uint32_t CpuCaps::myCpuCaps=0;
uint32_t CpuCaps::myCpuMask=0xffffffff;
uint32_t CpuCaps::myCpuManufacturer=0;

extern "C"
{
#include "libavutil/cpu.h"
}

#ifdef ADM_CPU_X86
  extern "C"
  {
  extern void adm_cpu_cpuid(int index, int *eax, int *ebx, int *ecx, int *edx);
  extern int  adm_cpu_cpuid_test(void);
  }
#endif



/**
 * 		\fn CpuCaps::init
 * 		\brief Detect the SIM capabilities of CPU, borrowed from lavcodec
 */
  void 	CpuCaps::init( void)
{
    printf("[CpuCaps] Checking CPU capabilities\n");
    myCpuCaps=0;
    myCpuMask=0xffffffff;
    myCpuManufacturer=0;
    bool available=true;

#ifdef ADM_CPU_X86
      int eax, ebx, ecx, edx;
      int max_std_level, max_ext_level;

  #ifdef ADM_CPU_X86_32
      available=adm_cpu_cpuid_test();
  #endif

      if(!available)
      {
        ADM_warning("CPUID not available\n");
        goto skipIt;
      }



   adm_cpu_cpuid(0, &max_std_level, &ebx, &ecx, &edx);
   char x86ManufacturerIDstring[13];    // string stored in EBX, EDX, ECX (in that order)
   memcpy(x86ManufacturerIDstring + 0, &ebx, 4);
   memcpy(x86ManufacturerIDstring + 4, &edx, 4);
   memcpy(x86ManufacturerIDstring + 8, &ecx, 4);
   x86ManufacturerIDstring[12] = 0;
   if (memcmp(x86ManufacturerIDstring, "GenuineIntel", 12) == 0)
       myCpuManufacturer = ADM_CPUMFR_INTEL;
   if (memcmp(x86ManufacturerIDstring, "AuthenticAMD", 12) == 0)
       myCpuManufacturer = ADM_CPUMFR_AMD;
   
   if(max_std_level >= 1)
   {
  	 int std_caps = 0;

       adm_cpu_cpuid(1, &eax, &ebx, &ecx, &std_caps);
       if (std_caps & (1<<23))
      	 myCpuCaps |= ADM_CPUCAP_MMX;
       if (std_caps & (1<<25))
      	 myCpuCaps |= ADM_CPUCAP_MMXEXT | ADM_CPUCAP_SSE;
       if (std_caps & (1<<26))
      	 myCpuCaps |= ADM_CPUCAP_SSE2;
       if (ecx & 1)
      	 myCpuCaps |= ADM_CPUCAP_SSE3;
       if (ecx & 0x00000200 )
      	 myCpuCaps |= ADM_CPUCAP_SSSE3;
       if (ecx & (1<<12) )
      	 myCpuCaps |= ADM_CPUCAP_FMA3;
       if (ecx & (1<<19) )
      	 myCpuCaps |= ADM_CPUCAP_SSE4;
       if (ecx & (1<<20) )
      	 myCpuCaps |= ADM_CPUCAP_SSE42;
       if (ecx & (1<<28) )
      	 myCpuCaps |= ADM_CPUCAP_AVX;
   }
   if(max_std_level >= 7)
   {
       ecx=0;
       adm_cpu_cpuid(7, &eax, &ebx, &ecx, &edx);
       if (ebx & (1<<5) )
      	 myCpuCaps |= ADM_CPUCAP_AVX2;
   }

   adm_cpu_cpuid(0x80000000,& max_ext_level,& ebx, &ecx,& edx);
   if(max_ext_level >= 0x80000001)
   {
  	 int ext_caps = 0;

       adm_cpu_cpuid(0x80000001, &eax,& ebx,& ecx,& ext_caps);
       if (ext_caps & (1<<31))
      	 myCpuCaps |= ADM_CPUCAP_3DNOW;
       if (ext_caps & (1<<30))
      	 myCpuCaps |= ADM_CPUCAP_3DNOWEXT;
       if (ext_caps & (1<<23))
      	 myCpuCaps |= ADM_CPUCAP_MMX;
       if (ext_caps & (1<<22))
      	 myCpuCaps |= ADM_CPUCAP_MMXEXT;
   }
skipIt:
  #define CHECK(x) if (myCpuCaps & ADM_CPUCAP_##x) { printf("\t\t"#x" detected"); \
                       if (!(myCpuMask & ADM_CPUCAP_##x)) printf(", but disabled"); \
                       printf("\n"); }
    CHECK(MMX)
    CHECK(3DNOW)
    CHECK(3DNOWEXT)
    CHECK(MMXEXT)
    CHECK(SSE)
    CHECK(SSE2)
    CHECK(SSE3)
    CHECK(SSSE3)
    CHECK(SSE4)
    CHECK(SSE42)
    CHECK(AVX)
    CHECK(AVX2)
    CHECK(FMA3)

    printf("[CpuCaps] CPU MFR-ID: %u, \"%s\"\n",myCpuManufacturer,x86ManufacturerIDstring);
#endif // X86
    ADM_info("[CpuCaps] End of CPU capabilities check (cpuCaps: 0x%08x, cpuMask: 0x%08x)\n",myCpuCaps,myCpuMask);
    return ;
}


//******************************************************


// Stolen from x264
int ADM_cpu_num_processors(void)
{
#if defined(_WIN32)
    return pthread_num_processors_np();
#elif defined(__APPLE__) || defined(ADM_BSD_FAMILY) && !defined(__HAIKU__)
    int np;

    size_t length = sizeof(np);

    if (sysctlbyname("hw.ncpu", &np, &length, NULL, 0))
        np = 1;

    return np;
#elif defined(__sun__)
	return sysconf( _SC_NPROCESSORS_ONLN );
#elif defined(__unix__) && !defined(__CYGWIN__)
    unsigned int bit;
    int np;

    cpu_set_t p_aff;
    memset( &p_aff, 0, sizeof(p_aff) );
    sched_getaffinity( 0, sizeof(p_aff), &p_aff );

    for( np = 0, bit = 0; bit < sizeof(p_aff); bit++ )
        np += (((uint8_t *)&p_aff)[bit / 8] >> (bit % 8)) & 1;

    return np;
#else
	return 1;
#endif
}
/**
 *
 * @param admMask
 * @return
 */
static int Cpu2Lav(uint32_t admMask)
{
   if(admMask==ADM_CPUCAP_ALL)
       return -1; // allow all
   int out=0;
 #define LAV_CPU_CAPS(x)    	if(admMask & ADM_CPUCAP_##x) out|=AV_CPU_FLAG_##x;

    	LAV_CPU_CAPS(MMX);
    	LAV_CPU_CAPS(MMXEXT);
    	LAV_CPU_CAPS(3DNOW);
    	LAV_CPU_CAPS(3DNOWEXT);
    	LAV_CPU_CAPS(SSE);
    	LAV_CPU_CAPS(SSE2);
    	LAV_CPU_CAPS(SSE3);
    	LAV_CPU_CAPS(SSSE3);
    	LAV_CPU_CAPS(SSE4);
    	LAV_CPU_CAPS(SSE42);
    	LAV_CPU_CAPS(AVX);
    	LAV_CPU_CAPS(AVX2);
    	LAV_CPU_CAPS(FMA3);
        return out;
}

/**
 *
 * @param mask
 * @return
 */
bool     CpuCaps::setMask(uint32_t mask)
{
    ADM_info("[CpuCaps] Setting mask to 0x%08x\n",mask);
    myCpuMask=mask;

    int lavCpuMask=Cpu2Lav(myCpuMask);
    //av_set_cpu_flags_mask(lavCpuMask); deprecated!
    lavCpuMask &= av_get_cpu_flags();
    ADM_info("[CpuCaps] Forcing lav cpu flags 0x%08x\n",lavCpuMask);
    av_force_cpu_flags(lavCpuMask);

    return true;
}
/**
 *
 * @param mask
 * @return
 */
uint32_t     CpuCaps::getMask( )
{
    return myCpuMask;
}
/**
 * \fn getCaps
 */
uint32_t CpuCaps::getCaps(void)
{
    return myCpuCaps;
}
extern "C"
{
extern void adm2_emms_yasm(void);
}
/**
 *
 *
 */
void ADM_emms()
{
#ifdef ADM_CPU_X86
    adm2_emms_yasm();
#endif
}

