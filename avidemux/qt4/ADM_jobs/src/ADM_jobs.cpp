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

#include "config.h"
#include "ADM_default.h"
#include "ADM_threads.h"
#include "ADM_coreJobs.h"
#include "ADM_memsupport.h"
#include "ADM_crashdump.h"
#include "ADM_win32.h"

void onexit( void );

extern uint8_t  quotaInit(void);
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

    installSigHandler();

    bool portableMode=isPortableMode(argc,argv);
    printf("*************************\n");
    printf("  Avidemux v" VERSION);
#if defined(ADM_SUBVERSION)
#define MKSTRING(x) x
     printf(" (%s)", MKSTRING(ADM_SUBVERSION));
#endif
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

#if defined(_WIN32)
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
#endif

	printf("\n");

#ifdef _WIN32
	char version[250];

	if (getWindowsVersion(version))
		printf("Operating System: %s\n", version);
#endif

#if defined(__USE_LARGEFILE) && defined(__USE_LARGEFILE64)
	printf("\nLarge file available: %d offset\n", __USE_FILE_OFFSET64);
#endif

	
	
#ifndef __APPLE__
    ADM_InitMemcpy();
#endif
	atexit(onexit);

#ifdef _WIN32
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


    ADM_initBaseDir(portableMode);
    // Init jobs
    ADMJob::jobInit();
    jobRun(argc,argv);

	uninstallSigHandler();

    printf("Normal exit\n");
    return 0;
}

void onexit( void )
{
    printf("Cleaning up\n");
    ADMJob::jobShutDown();   
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
