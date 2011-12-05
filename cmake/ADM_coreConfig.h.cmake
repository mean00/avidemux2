#ifndef ADM_CORE_H
#define ADM_CORE_H

#define ADM_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}"
#define ADM_RELATIVE_LIB_DIR "${AVIDEMUX_RELATIVE_LIB_DIR}"
#define ADM_PLUGIN_DIR "${ADM_PLUGIN_DIR}"

// GCC - CPU
#cmakedefine ADM_BIG_ENDIAN
#cmakedefine ADM_CPU_64BIT
#cmakedefine ADM_CPU_ALTIVEC
#cmakedefine ADM_CPU_ARMEL
#cmakedefine ADM_CPU_DCBZL
#cmakedefine ADM_CPU_PPC
#cmakedefine ADM_CPU_MMX2
#cmakedefine ADM_CPU_SSSE3
#cmakedefine ADM_CPU_X86
#cmakedefine ADM_CPU_X86_32
#cmakedefine ADM_CPU_X86_64

// GCC - Operating System
#cmakedefine ADM_BSD_FAMILY

// use vdpau h264 hw decoding 
#cmakedefine USE_VDPAU

// 'gettimeofday' function is present
#cmakedefine HAVE_GETTIMEOFDAY

// Presence of header files
#cmakedefine HAVE_BYTESWAP_H
#cmakedefine HAVE_INTTYPES_H
#cmakedefine HAVE_MALLOC_H
#cmakedefine HAVE_STDINT_H
#cmakedefine HAVE_SYS_TYPES_H

#ifdef __MINGW32__
#define rindex strrchr
#define index strchr
#if !${USE_FTELLO}
        #define ftello ftello64 // not defined on every mingw64_w32 version (e.g. set 2011-11-03 does not have it)
        #define fseeko fseeko64
#endif // FTELLO
#endif

#endif
