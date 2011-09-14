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
#include <execinfo.h>

#include "ADM_default.h"

// Our callback to give UI formatted informations....
static ADM_saveFunction *mysaveFunction=NULL;
static ADM_fatalFunction *myFatalFunction=NULL;

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

void ADM_backTrack(const char *info,int lineno,const char *file)
{
	char wholeStuff[2048];
    char buffer[4096];
    char in[2048];
	void *stack[20];
	char **functions;
	int count, i;

	wholeStuff[0]=0;

	if(mysaveFunction)
		mysaveFunction();

#ifndef(__HAIKU__)
	printf("\n*********** BACKTRACK **************\n");

	count = backtrace(stack, 20);
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
            if(status) strcpy(buffer,in);
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
}
#endif
//EOF
