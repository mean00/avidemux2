/** *************************************************************************
    \fn ADM_misc.h
    \brief Handle string/log/... related functions
                      
    copyright            : (C) 2008 by mean
    
 ***************************************************************************/

#ifndef ADM_MISC_H
#define ADM_MISC_H

#include "ADM_coreConfig.h"
#include "ADM_inttype.h"

typedef struct 
{
  uint32_t hours,minutes,seconds; 
} ADM_date;

void            TLK_getDate(ADM_date *date);
// /dir/file.ext -> /dir/file and ext returned values are copies
void            ADM_PathSplit(const char *str, char **root, char **ext);
// Returns path only /foo/bar.avi -> /foo INPLACE, no copy done
void	        ADM_PathStripName(char *str);
// Get the filename without path. /foo/bar.avi -> bar.avi INPLACE, NO COPY
const char      *ADM_GetFileName(const char *str);
//  Canonize the path, returns a copy of the absolute path given as parameter
char            *ADM_PathCanonize(const char *tmpname);
// change to lower case in place the string
void            ADM_LowerCase(char *string);

uint32_t        getTime( int called );
uint32_t 	    getTimeOfTheDay(void);

uint64_t        ADM_getSecondsSinceEpoch(void);
const char      *ADM_epochToString(uint64_t epoch);

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

	extern "C"
	{
		typedef struct timespec
		{
			time_t tv_sec;
			long int tv_nsec;
		};
	};
#		endif

#	define TIMZ int
#	endif

	extern "C" void gettimeofday(struct timeval *p, TIMZ *tz);
#endif

#ifdef _WIN32
	#define PRIO_MIN -20
	#define PRIO_MAX 20
	#define PRIO_PROCESS 0

	int getpriority(int which, int who);
	int setpriority(int which, int who, int value);
#else
	#include <sys/resource.h>
#endif

bool shutdown(void);

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
