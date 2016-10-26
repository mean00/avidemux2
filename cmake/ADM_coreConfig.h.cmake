#ifndef ADM_CORE_H
#define ADM_CORE_H

#define ADM_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}"
#define ADM_RELATIVE_LIB_DIR "${AVIDEMUX_RELATIVE_LIB_DIR}"
#define ADM_PLUGIN_DIR "${ADM_PLUGIN_DIR}"

// GCC - CPU
#cmakedefine ADM_BIG_ENDIAN
#cmakedefine ADM_CPU_64BIT
#cmakedefine ADM_CPU_ARMEL
#cmakedefine ADM_CPU_X86
#cmakedefine ADM_CPU_X86_32
#cmakedefine ADM_CPU_X86_64

// GCC - Operating System
#cmakedefine ADM_BSD_FAMILY

// use nvidia hw encoding 
#cmakedefine USE_NVENC
// use vdpau h264 hw decoding 
#cmakedefine USE_VDPAU
// use xvba h264 hw decoding 
#cmakedefine USE_XVBA
// use libva h264 hw decoding 
#cmakedefine USE_LIBVA
// use dxva2 hw decoding 
#cmakedefine USE_DXVA2

// 'gettimeofday' function is present
#cmakedefine HAVE_GETTIMEOFDAY

// Presence of header files
#cmakedefine HAVE_INTTYPES_H   1
#cmakedefine HAVE_STDINT_H     1
#cmakedefine HAVE_SYS_TYPES_H

#ifdef _MSC_VER
#	define ftello _ftelli64
#	define fseeko _fseeki64
#	define snprintf _snprintf
#	define strcasecmp(x, y) _stricmp(x, y)
#elif defined(__MINGW32__)
#	define rindex strrchr
#	define index strchr

#	if !${USE_FTELLO}
#		define ftello ftello64 // not defined on every mingw64_w32 version (e.g. set 2011-11-03 does not have it)
#		define fseeko fseeko64
#	endif // FTELLO
#endif

#if defined(ADM_CPU_X86_32) && defined(__GNUC__)
#    define attribute_align_arg __attribute__((force_align_arg_pointer))
#else
#    define attribute_align_arg
#endif

/* use Nvwa memory leak detector */
#cmakedefine FIND_LEAKS

#endif
