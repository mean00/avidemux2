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
#include <math.h>

#ifdef __MINGW32__
#include <windows.h>
#include <excpt.h>
#include <imagehlp.h>
#endif

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

static void saveCrashProject(void);
extern char *ADM_getBaseDir(void);
extern void A_parseECMAScript(const char *name);

#ifdef __APPLE__
void installSigHandler() {}

void ADM_backTrack(const char *info,int lineno,const char *file)
{
	char bfr[1024];

	if (mysaveFunction)
		mysaveFunction();

	snprintf(bfr,1024,"%s\n file %s, line %d\n", info, file, lineno);

	if(myFatalFunction)
		myFatalFunction("Crash",bfr);

	exit(-1);
}
#elif defined(__MINGW32__)
typedef struct STACK_FRAME
{
    STACK_FRAME* ebp;	// address of the calling function frame
    uint8_t* retAddr;	// return address
    uint32_t param[0];	// parameter list (could be empty)
} STACK_FRAME;

static void dumpFrame(void* processId, void* frameAddr)
{
	MEMORY_BASIC_INFORMATION mbi;
	char moduleName[MAX_PATH];
	HMODULE moduleAddr;
	DWORD symDisplacement;
	IMAGEHLP_SYMBOL* pSymbol;

	if (VirtualQuery(frameAddr, &mbi, sizeof(mbi)))
	{
		moduleName[0] = 0;
		moduleAddr = (HMODULE)mbi.AllocationBase;
		
		GetModuleFileName(moduleAddr, moduleName, MAX_PATH);

		printf("%s(", moduleName);

		if (SymGetSymFromAddr(processId, (uint32_t)frameAddr, &symDisplacement, pSymbol))
			printf("%s", pSymbol->Name);
		else
			printf("<unknown>");

		printf("+0x%X) [0x%08X]\n", (uint32_t)frameAddr - (uint32_t)moduleAddr, frameAddr);

		fflush(stdout);
	}
}
	
static void dumpExceptionInfo(void* processId, struct _EXCEPTION_RECORD* pExceptionRec, struct _CONTEXT* pContextRecord)
{
	printf("\n*********** EXCEPTION **************\n");
	printf("Registers:\n");
	printf("EAX: %08X  EBX: %08X  ECX: %08X  EDX: %08X  ESI: %08X\n", pContextRecord->Eax, pContextRecord->Ebx, pContextRecord->Ecx, pContextRecord->Edx, pContextRecord->Esi);
	printf("EDI: %08X  ESP: %08X  EBP: %08X  EIP: %08X  EFlags: %08X\n\n", pContextRecord->Edi, pContextRecord->Esp, pContextRecord->Ebp, pContextRecord->Eip, pContextRecord->EFlags);

	printf("Exception Code: ");

	switch (pExceptionRec->ExceptionCode)
	{
		case EXCEPTION_ACCESS_VIOLATION:
			printf("EXCEPTION_ACCESS_VIOLATION");
			break;
		case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
			printf("EXCEPTION_ARRAY_BOUNDS_EXCEEDED");
			break;
		case EXCEPTION_BREAKPOINT:
			printf("EXCEPTION_BREAKPOINT");
			break;
		case EXCEPTION_DATATYPE_MISALIGNMENT:
			printf("EXCEPTION_DATATYPE_MISALIGNMENT");
			break;
		case EXCEPTION_FLT_DENORMAL_OPERAND:
			printf("EXCEPTION_FLT_DENORMAL_OPERAND");
			break;
		case EXCEPTION_FLT_DIVIDE_BY_ZERO:
			printf("EXCEPTION_FLT_DIVIDE_BY_ZERO");
			break;
		case EXCEPTION_FLT_INEXACT_RESULT:
			printf("EXCEPTION_FLT_INEXACT_RESULT");
			break;
		case EXCEPTION_FLT_INVALID_OPERATION:
			printf("EXCEPTION_FLT_INVALID_OPERATION");
			break;
		case EXCEPTION_FLT_OVERFLOW:
			printf("EXCEPTION_FLT_OVERFLOW");
			break;
		case EXCEPTION_FLT_STACK_CHECK:
			printf("EXCEPTION_FLT_STACK_CHECK");
			break;
		case EXCEPTION_FLT_UNDERFLOW:
			printf("EXCEPTION_FLT_UNDERFLOW");
			break;
		case EXCEPTION_ILLEGAL_INSTRUCTION:
			printf("EXCEPTION_ILLEGAL_INSTRUCTION");
			break;
		case EXCEPTION_IN_PAGE_ERROR:
			printf("EXCEPTION_IN_PAGE_ERROR");
			break;
		case EXCEPTION_INT_DIVIDE_BY_ZERO:
			printf("EXCEPTION_INT_DIVIDE_BY_ZERO");
			break;
		case EXCEPTION_INT_OVERFLOW:
			printf("EXCEPTION_INT_OVERFLOW");
			break;
		case EXCEPTION_INVALID_DISPOSITION:
			printf("EXCEPTION_INVALID_DISPOSITION");
			break;
		case EXCEPTION_NONCONTINUABLE_EXCEPTION:
			printf("EXCEPTION_NONCONTINUABLE_EXCEPTION");
			break;
		case EXCEPTION_PRIV_INSTRUCTION:
			printf("EXCEPTION_PRIV_INSTRUCTION");
			break;
		case EXCEPTION_SINGLE_STEP:
			printf("EXCEPTION_SINGLE_STEP");
			break;
		case EXCEPTION_STACK_OVERFLOW:
			printf("EXCEPTION_STACK_OVERFLOW");
			break;
		default:
			printf("UNKNOWN");
	}
	
	printf(" (%08X)\n", pExceptionRec->ExceptionCode);
	printf("Exception Flags: %08X\n", pExceptionRec->ExceptionFlags);

	printf("\nOrigin:\n");
	dumpFrame(processId, (void*)pContextRecord->Eip);
	printf("*********** EXCEPTION **************\n");
	fflush(stdout);
}

extern "C"
{
void dumpBackTrace(void* processId)
{
	if (!processId)
		processId = GetCurrentProcess();

	const int maxAddrCount = 32;

	printf("\n*********** BACKTRACE **************\n");

	// Get frame address using builtin GCC function.
	STACK_FRAME* stackFrame = (STACK_FRAME*)__builtin_frame_address(0);

    for (int retAddrCount = 0; (retAddrCount < maxAddrCount) && !IsBadReadPtr(stackFrame, sizeof(STACK_FRAME)) && !IsBadCodePtr(FARPROC(stackFrame->retAddr)); retAddrCount++, stackFrame = stackFrame->ebp)
    {
		printf("Frame %2d: ", retAddrCount);
		dumpFrame(processId, stackFrame->retAddr);
		fflush(stdout);
    }

	printf("*********** BACKTRACE **************\n\n");
}
}

void ADM_backTrack(const char *info,int lineno,const char *file)
{	
	fflush(stderr);
	fflush(stdout);

	if (mysaveFunction)
		mysaveFunction();

	if (myFatalFunction)
		myFatalFunction("Crash", "Press OK to build crash info");

	void* currentProcessId = GetCurrentProcess();

	SymInitialize(currentProcessId, NULL, TRUE);
	dumpBackTrace(currentProcessId);
	SymCleanup(currentProcessId);

	printf("Assert failed at file %s, line %d\n\n",file,lineno);

	exit(-1);
}

EXCEPTION_DISPOSITION exceptionHandler(struct _EXCEPTION_RECORD* pExceptionRec, void* pEstablisherFrame, struct _CONTEXT* pContextRecord, void* pDispatcherContext)
{
	fflush(stderr);
	fflush(stdout);
	static int running=0;

	if(running)
		exit(1);

	running=1;

	if (mysaveFunction)
		mysaveFunction();

	if (myFatalFunction)
		myFatalFunction("Crash", "Press OK to build crash info");

	void* currentProcessId = GetCurrentProcess();

	SymInitialize(currentProcessId, NULL, TRUE);

	dumpExceptionInfo(currentProcessId, pExceptionRec, pContextRecord);
	fflush(stdout);
	dumpBackTrace(currentProcessId);

	SymCleanup(currentProcessId);

	exit(1);
}
#else
#include <signal.h>

#ifndef __CYGWIN__
#include <execinfo.h>
#endif

void sig_segfault_handler(int signo);
void installSigHandler()
{
    signal(11, sig_segfault_handler); // show stacktrace on default
}

static int lenCount(uint8_t *start,uint8_t *end,int *d)
{
  int val=0;
  int digit=0;
  *d=0;
  while(*start>='0' && *start<='9' && start<end) 
  {
    val=val*10+*start-'0'; 
    start++;
    digit++;
  }
  *d=digit;
  return val;
}

static int decodeOne(uint8_t *start, uint8_t *end,int *cons)
{
  *cons=0;
  int len,digit;
  uint8_t *org=start;
  if(start+2>=end) return 0;
  switch(*start )
  {
    case 'Z':
    case 'P':  
      
        len=lenCount(start+1,end,&digit);
        start+=1+digit;
        for(int z=0;z<len;z++) printf("%c",*start++);
        break;
  }
  return (int)((uint64_t)start-(uint64_t)org);
}
static int demangle(int i,  uint8_t *string)
{
 // Search 1st (
  if(!string) return 0;
  int len=strlen((char *)string);
  if(!len) return 0;
   
  uint8_t *end=string+len;
  uint8_t *start=string;
  
  while(*start!='(' && start+3<end) start++;
  if(*start!='(') return 0;
  start++;
  
  //  _qt(_Z9crashTestP9JSContextP8JSObjectjPlS3_+0) [0x4acf80]
  
  if(*start!='_' || start[1]!='Z')
  {
    return 0;
  }
  // Seems good !
  start++;
  start++;  
  int digit;
  // Function name..
  int l=lenCount(start,end,&digit);
  printf("\t<");
  for(int i=0;i<l;i++)
  {
    printf("%c",start[digit+i]); 
  }
  printf(">(");
  start+=digit+l;
  
  // Parama
  int first=0;
  while(start+2<end && *start=='P')
  {
    if(!first)  first=1;
    else
        printf(",");
    
      
    start++;
    l=lenCount(start,end,&digit);
    for(int i=0;i<l;i++)
    {
      printf("%c",start[digit+i]); 
    }
    start+=digit+l;
  }
  printf(")\n");
  return 1;
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
	void *stack[20];
	char **functions;
	int count, i;

	wholeStuff[0]=0;

	if(mysaveFunction)
		mysaveFunction();

#ifndef __CYGWIN__
	printf("\n*********** BACKTRACK **************\n");

	count = backtrace(stack, 20);
	functions = backtrace_symbols(stack, count);
	sprintf(wholeStuff,"%s\n at line %d, file %s",info,lineno,file);

	for (i=0; i < count; i++) 
	{
		printf("Frame %2d: %s \n", i, functions[i]);
		demangle(i,(uint8_t *)functions[i]);
		strcat(wholeStuff,functions[i]);
		strcat(wholeStuff,"\n");
	}

	printf("*********** BACKTRACK **************\n");

	if(myFatalFunction)
		myFatalFunction("Crash", wholeStuff); // FIXME
#endif	//__CYGWIN__

	exit(-1); // _exit(1) ???
}
#endif
//EOF
