/***************************************************************************
  Try to display interesting crash dump

    copyright            : (C) 2007 by mean, (C) 2007 Gruntster
    email                : fixounet@free.fr
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <cxxabi.h>
#include <signal.h>
#if defined(__sun__)
#include <dlfcn.h>
#include <strings.h>
#include <ucontext.h>
#include <sys/stack.h>
#ifdef _LP64
#define	_ELF64
#endif
#include <sys/machelf.h>
#else
#include <execinfo.h>
#endif

#include "ADM_default.h"

// Our callback to give UI formatted informations....
static ADM_saveFunction *mysaveFunction=NULL;
static ADM_fatalFunction *myFatalFunction=NULL;
static sighandler_t      oldSignalHandler;
void sig_segfault_handler(int signo);
/**
        \fn ADM_setCrashHook
        \brief install crash handlers (save + display)
*/
void ADM_setCrashHook(ADM_saveFunction *save, ADM_fatalFunction *fatal)
{
        mysaveFunction=save;
        myFatalFunction=fatal;
}
/**
    \fn installSigHandler
*/
void installSigHandler(void)
{
    oldSignalHandler=signal(11, sig_segfault_handler); // show stacktrace on default
}
void uninstallSigHandler(void)
{
    ADM_info("Removing signal handler\n");
    signal(11, SIG_DFL); 
}

/**
      \fn sig_segfault_handler
      \brief our segfault handler
*/
void sig_segfault_handler(int signo)
{
     static int running=0;
      if(running)
      {
        signo=0;
        exit(1);
      }
      running=0;
      ADM_backTrack("Segfault",0,"??");
}

#if defined(__sun__)
static const int maxSize = 2048;

static void addr2sym(void* pc, char* buffer, int size)
{
	Dl_info info;
	Sym* sym = (Sym*)0;
	static size_t dsize = maxSize - 1;
	static char demangled[maxSize];
	int dstatus = 0;

	if (dladdr1(pc, &info, (void**)&sym, RTLD_DL_SYMENT) == 0)
	{
		snprintf(buffer, size, "[0x%p]", pc);
	}

	if ((info.dli_fname != NULL && info.dli_sname != NULL) &&
	    (((uintptr_t)pc - (uintptr_t)info.dli_saddr) < sym->st_size))
	{
		__cxxabiv1::__cxa_demangle(info.dli_sname,demangled,&dsize,&dstatus);
		snprintf(buffer, size, "%s'%s+0x%x [0x%p]",
				 info.dli_fname,
				 demangled,
				 (unsigned long)pc - (unsigned long)info.dli_saddr,
				 pc);
	}
	else
	{
		snprintf(buffer, size, "%s'0x%p [0x%p]",
				 info.dli_fname,
				 (unsigned long)pc - (unsigned long)info.dli_fbase,
				 pc);
	}

	return;
}

static void printFrame(int fd, const char* format, ...)
{
	va_list ap;
	static char buffer[maxSize];

	va_start(ap, format);
	(void)vsnprintf(buffer, sizeof (buffer), format, ap);
	va_end(ap);

	(void)write(fd, buffer, strlen(buffer));
}

static int printStack(uintptr_t pc, int signo, void *arg)
{

	static char buffer[maxSize];
	char sigbuf[SIG2STR_MAX];


	int filenum = (intptr_t)arg;

	addr2sym((void *)pc, buffer, sizeof (buffer));

	if (signo) {
		sigbuf[0] = '?';
		sigbuf[1] = 0;

		(void) sig2str(signo, sigbuf);

		printFrame(filenum, "%s [Signal %d (%s)]\n",
		    buffer, (ulong_t)signo, sigbuf);
	} else
		printFrame(filenum, "%s\n", buffer);

	return (0);
}

static int backtrace(int fd)
{
	int rc = -1;
	ucontext_t u;

	if (getcontext(&u) >= 0)
	{
		rc = walkcontext(&u, printStack, (void*)(intptr_t)fd);
	}
	return(rc);
}
#endif /* : defined(__sun__) */

void ADM_backTrack(const char *info,int lineno,const char *file)
{
	if(mysaveFunction)
		mysaveFunction();
#define MAX_BACKTRACK 30
#if !defined(__HAIKU__) && !defined(__sun__)
    char wholeStuff[2048];
    char buffer[4096];
    char in[2048];
	void *stack[MAX_BACKTRACK+1];
	char **functions;
	int count, i;
	wholeStuff[0]=0;

	printf("\n*********** BACKTRACK **************\n");

	count = backtrace(stack, MAX_BACKTRACK);
	functions = backtrace_symbols(stack, count);
	sprintf(wholeStuff,"%s\n at line %d, file %s",info,lineno,file);
        int status;
        size_t size=2047;
    // it looks like that xxxx (functionName+0x***) XXXX
	for (i=0; i < count; i++)
	{
            char *s=strstr(functions[i],"(");
            buffer[0]=0;
            if(s && strstr(s+1,"+"))
            {
                strcpy(in,s+1);
                char *e=strstr(in,"+");
                *e=0;
                __cxxabiv1::__cxa_demangle(in,buffer,&size,&status);
                if(status) 
                    strcpy(buffer,in);
            }else
                strcpy(buffer,functions[i]);
            printf("%s:%d:<%s>:%d\n",functions[i],i,buffer,status);
            strcat(wholeStuff,buffer);
            strcat(wholeStuff,"\n");
        }
	printf("*********** BACKTRACK **************\n");

	if(myFatalFunction)
		myFatalFunction("Crash", wholeStuff); // FIXME

	exit(-1); // _exit(1) ???
#endif
}
//EOF
