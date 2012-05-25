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

#if defined(_WIN64)
extern LONG WINAPI ExceptionFilter(struct _EXCEPTION_POINTERS *exceptionInfo);
#elif defined(_WIN32)
extern EXCEPTION_DISPOSITION ExceptionHandler(struct _EXCEPTION_RECORD *exceptionRecord, void *establisherFrame, struct _CONTEXT *contextRecord, void *dispatcherContext);
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

#if !defined(NDEBUG) && defined(FIND_LEAKS)
extern const char* new_progname;
#endif
static bool isPortableMode(int argc, char *argv[]);
/**
    \fn main
*/
int main(int argc, char *argv[])
{
#if defined(_WIN32)
	redirectStdoutToFile();
#endif

#if !defined(NDEBUG) && defined(FIND_LEAKS)
	new_progname = argv[0];
#endif

#ifndef __MINGW32__
	// thx smurf uk :)
    installSigHandler();
#endif
    bool portableMode=isPortableMode(argc,argv);
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
#ifndef __APPLE__
    ADM_InitMemcpy();
#endif
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


#if defined(_WIN64)
	SetUnhandledExceptionFilter(ExceptionFilter);
#elif defined(_WIN32)
	__try1(ExceptionHandler);
#endif
    ADM_initBaseDir(portableMode);
    // Init jobs
    ADM_jobInit();
    jobRun(argc,argv);

#if defined(_WIN32) && defined(_X86_)
	__except1;
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
}

/**
    \fn isPortableMode
    \brief returns true if we are in portable mode
*/
bool isPortableMode(int argc, char *argv[])
{
	bool portableMode = false;
    std::string mySelf=argv[0];
    // if the name ends by "_portable.exe" => portable
    int match=mySelf.find("portable");
    if(match!=-1)
    {
        ADM_info("Portable mode\n");
        return true;
    }

	for (int i = 0; i < argc; i++)
	{
		if (strcmp(argv[i], "--portable") == 0)
		{
			portableMode = true;
			break;
		}
	}

	return portableMode;
}
//EOF
