/**
         \file ADM_jobs.cpp
         \brief external job control
         \author mean fixounet@free.fr (c) 2010
*/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "ADM_cpp.h"

#ifdef __MINGW32__
#include <windows.h>
#include <excpt.h>
#endif

#include "config.h"
#include "ADM_default.h"
#include "ADM_threads.h"
#include "ADM_coreJobs.h"
extern "C" {
     extern uint8_t     ADM_InitMemcpy(void);
};

void onexit( void );
extern uint8_t initGUI( void );
extern void destroyGUI(void);
extern uint8_t initFileSelector(void);
extern void ADM_memStat( void );
extern void ADM_memStatInit( void );
extern void ADM_memStatEnd( void );
extern void InitFactory(void);
extern void InitCoreToolkit(void);
#ifdef __MINGW32__
extern EXCEPTION_DISPOSITION exceptionHandler(struct _EXCEPTION_RECORD* pExceptionRec, void* pEstablisherFrame, struct _CONTEXT* pContextRecord, void* pDispatcherContext);
#else
extern void installSigHandler(void);
#endif

#ifdef __WIN32
extern bool getWindowsVersion(char* version);
extern void redirectStdoutToFile(void);
#endif

extern uint8_t  quotaInit(void);
extern uint8_t win32_netInit(void);
extern bool jobRun(int ac, char **av);


int main(int argc, char *argv[])
{
#if defined(__WIN32) && defined(USE_SDL)
//	redirectStdoutToFile();
#endif

#if defined(ADM_DEBUG) && defined(FIND_LEAKS)
	new_progname = argv[0];
#endif

#ifndef __MINGW32__
	// thx smurf uk :)
    installSigHandler();
#endif

    printf("*************************\n");
    printf("  Avidemux v" VERSION);
  	if(ADM_SUBVERSION)
	{
		printf(" (r%04u)", ADM_SUBVERSION);
	}
    printf("\n*************************\n");
    printf(" http://www.avidemux.org\n");
    printf(" Code      : Mean, JSC, Gruntster \n");
    printf(" GFX       : Nestor Di , nestordi@augcyl.org\n");
    printf(" Design    : Jakub Misak\n");
    printf(" FreeBSD   : Anish Mistry, amistry@am-productions.biz\n");
    printf(" Audio     : Mihail Zenkov\n");
    printf(" MacOsX    : Kuisathaverat\n");
    printf(" Win32     : Gruntster\n\n");

#ifdef __GNUC__
	printf("Compiler: GCC %s\n", __VERSION__);
#endif

	printf("Build Target: ");

#if defined(__WIN32)
	printf("Microsoft Windows");
#elif defined(__APPLE__)
	printf("Apple");
#else
	printf("Linux");
#endif

#if defined(ADM_CPU_X86_32)
	printf(" (x86)");
#elif defined(ADM_CPU_X86_64)
	printf(" (x86-64)");
#elif defined(ADM_CPU_PPC)
	printf(" (PowerPC)");
#endif

	printf("\n");

#ifdef __WIN32
	char version[250];

	if (getWindowsVersion(version))
		printf("Operating System: %s\n", version);
#endif

#if defined(__USE_LARGEFILE) && defined(__USE_LARGEFILE64)
	printf("\nLarge file available: %d offset\n", __USE_FILE_OFFSET64);
#endif

	// Start counting memory
	ADM_memStatInit();
    ADM_InitMemcpy();

	atexit(onexit);

#ifdef __MINGW32__
    win32_netInit();
#endif

#if 0
    // Hook our UI...
    InitFactory();
    InitCoreToolkit();
    initFileSelector();
#endif

	// Load .avidemuxrc
    quotaInit();


#ifdef __MINGW32__
	__try1(exceptionHandler);
#endif

    // Init jobs
    ADM_jobInit();
    jobRun(argc,argv);

#ifdef __MINGW32__
	__except1(exceptionHandler);
#endif

    printf("Normal exit\n");
    return 0;
}

void onexit( void )
{
	printf("Cleaning up\n");
    ADM_memStat();
    ADM_memStatEnd(); 
    ADM_jobShutDown();   
    ADM_info("\nGoodbye...\n\n");

#if defined(ADM_DEBUG) && defined(FIND_LEAKS)
	check_leaks();
#endif
}
//EOF
