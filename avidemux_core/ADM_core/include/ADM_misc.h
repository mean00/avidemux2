/** *************************************************************************
    \fn ADM_misc.h
    \brief Handle string/log/... related functions
                      
    copyright            : (C) 2008 by mean
    
 ***************************************************************************/

#ifndef ADM_MISC_H
#define ADM_MISC_H

#include "ADM_core6_export.h"
#include "ADM_coreConfig.h"
#include "ADM_inttype.h"
#include <string>

typedef struct 
{
  uint32_t hours,minutes,seconds; 
} ADM_date;

ADM_CORE6_EXPORT const ADM_date     ADM_getCurrentDate();
// /dir/file.ext -> /dir/file and ext returned values are copies
ADM_CORE6_EXPORT void               ADM_PathSplit(const std::string &in,std::string &root, std::string &ext);
// Returns path only /foo/bar.avi -> /foo , INPLACE!
ADM_CORE6_EXPORT std::string        ADM_extractPath(const std::string &str);
// Get the filename without path. /foo/bar.avi -> bar.avi
ADM_CORE6_EXPORT const std::string  ADM_getFileName(const std::string &str);
//  Canonize the path, returns a copy of the absolute path given as parameter
ADM_CORE6_EXPORT char              *ADM_PathCanonize(const char *tmpname);
// change to lower case in place the string
ADM_CORE6_EXPORT void               ADM_lowerCase(std::string &st);
//
ADM_CORE6_EXPORT uint64_t           ADM_getSecondsSinceEpoch(void);
ADM_CORE6_EXPORT const char        *ADM_epochToString(uint64_t epoch);

#ifdef HAVE_GETTIMEOFDAY
	#include <sys/time.h>

	#define TIMZ struct timezone
#else
#	ifdef _WIN32
#		include <WinSock.h>
#	endif

#	ifndef HAVE_STRUCT_TIMESPEC
#	define HAVE_STRUCT_TIMESPEC

#		ifndef _TIMESPEC_DEFINED
#		define _TIMESPEC_DEFINED
        #ifndef _MSC_VER

	extern "C"
	{
		typedef struct timespec
		{
			time_t tv_sec;
			long int tv_nsec;
		};
	};
        #endif
#		endif

#	define TIMZ int
#	endif

	extern "C" void gettimeofday(struct timeval *p, TIMZ *tz);
#endif

#ifdef _WIN32
	#define PRIO_MIN -20
	#define PRIO_MAX 20
	#define PRIO_PROCESS 0

	ADM_CORE6_EXPORT int getpriority(int which, int who);
	ADM_CORE6_EXPORT int setpriority(int which, int who, int value);
#else
	#include <sys/resource.h>
#endif

bool ADM_shutdown(void);

#ifdef ADM_BIG_ENDIAN	
	#define R64 ADM_swap64
	#define R32 ADM_swap32
	#define R16 ADM_swap16
#else
	#define R64(x) (x)
	#define R32(x) (x) 
	#define R16(x) (x) 
#endif
#endif
