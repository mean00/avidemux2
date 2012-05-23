#ifndef ADM_CORE_H
#define ADM_CORE_H

#define ADM_INSTALL_DIR "/usr"
#define ADM_RELATIVE_LIB_DIR "lib"
#define ADM_PLUGIN_DIR "ADM_plugins6"

// GCC - CPU
/* #undef ADM_BIG_ENDIAN */
#define ADM_CPU_64BIT
/* #undef ADM_CPU_ALTIVEC */
/* #undef ADM_CPU_ARMEL */
/* #undef ADM_CPU_DCBZL */
/* #undef ADM_CPU_PPC */
#define ADM_CPU_MMX2
#define ADM_CPU_SSSE3
#define ADM_CPU_X86
/* #undef ADM_CPU_X86_32 */
#define ADM_CPU_X86_64

// GCC - Operating System
/* #undef ADM_BSD_FAMILY */

// use vdpau h264 hw decoding 
#define USE_VDPAU

// 'gettimeofday' function is present
#define HAVE_GETTIMEOFDAY

// Presence of header files
#define HAVE_BYTESWAP_H
#define HAVE_INTTYPES_H   1
#define HAVE_MALLOC_H
#define HAVE_STDINT_H     1
#define HAVE_SYS_TYPES_H

#ifdef __MINGW32__
#define rindex strrchr
#define index strchr
#if !1
        #define ftello ftello64 // not defined on every mingw64_w32 version (e.g. set 2011-11-03 does not have it)
        #define fseeko fseeko64
#endif // FTELLO
#endif

#define USE_SPIDERMONKEY
#define USE_TINYPY

/* use Nvwa memory leak detector */
/* #undef FIND_LEAKS */

#endif
