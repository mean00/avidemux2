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
#include <string>
#ifdef __MINGW32__
#include <windows.h>
#include <excpt.h>
#include <imagehlp.h>
#else
#include <cxxabi.h>
#endif

#include "ADM_default.h"
using namespace std;
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
#ifdef ADM_CPU_X86_64
#define ARCH_DWORD DWORD64
#else
#define ARCH_DWORD DWORD
#endif

	MEMORY_BASIC_INFORMATION mbi;
	char moduleName[MAX_PATH];
	HMODULE moduleAddr;
	ARCH_DWORD symDisplacement;
	IMAGEHLP_SYMBOL* pSymbol;

	if (VirtualQuery(frameAddr, &mbi, sizeof(mbi)))
	{
		moduleName[0] = 0;
		moduleAddr = (HMODULE)mbi.AllocationBase;
		
		GetModuleFileName(moduleAddr, moduleName, MAX_PATH);

		printf("%s(", moduleName);

		if (SymGetSymFromAddr(processId, (ARCH_DWORD)frameAddr, &symDisplacement, pSymbol))
			printf("%s", pSymbol->Name);
		else
			printf("<unknown>");

		printf("+0x%X) [0x%08X]\n", (ARCH_DWORD)frameAddr - (ARCH_DWORD)moduleAddr, frameAddr);

		fflush(stdout);
	}
}
	
static void dumpExceptionInfo(void* processId, struct _EXCEPTION_RECORD* pExceptionRec, struct _CONTEXT* pContextRecord)
{
	printf("\n*********** EXCEPTION **************\n");
	printf("Registers:\n");
#ifdef ADM_CPU_X86_64
	printf("RAX: %08X  RBX: %08X  RCX: %08X  RDX: %08X  RSI: %08X  RDI: %08X  RSP: %08X  RBP: %08X\n", pContextRecord->Rax, pContextRecord->Rbx, pContextRecord->Rcx, pContextRecord->Rdx, pContextRecord->Rsi, pContextRecord->Rdi, pContextRecord->Rsp, pContextRecord->Rbp);
	printf("R8: %08X  R9: %08X  R10: %08X  R11: %08X  R12: %08X  R13: %08X  R14: %08X  R15: %08X\n", pContextRecord->R8, pContextRecord->R9, pContextRecord->R10, pContextRecord->R11, pContextRecord->R12, pContextRecord->R13, pContextRecord->R14, pContextRecord->R15);
	printf("RIP: %08X  EFlags: %08X\n\n", pContextRecord->Rip, pContextRecord->EFlags);
#else
	printf("EAX: %08X  EBX: %08X  ECX: %08X  EDX: %08X  ESI: %08X\n", pContextRecord->Eax, pContextRecord->Ebx, pContextRecord->Ecx, pContextRecord->Edx, pContextRecord->Esi);
	printf("EDI: %08X  ESP: %08X  EBP: %08X  EIP: %08X  EFlags: %08X\n\n", pContextRecord->Edi, pContextRecord->Esp, pContextRecord->Ebp, pContextRecord->Eip, pContextRecord->EFlags);
#endif

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

#ifdef ADM_CPU_X86_64
	dumpFrame(processId, (void*)pContextRecord->Rip);
#else
	dumpFrame(processId, (void*)pContextRecord->Eip);
#endif

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

#if !defined( __CYGWIN__) && !defined(__HAIKU__)
#include <execinfo.h>
#endif

void sig_segfault_handler(int signo);
void installSigHandler()
{
    signal(11, sig_segfault_handler); // show stacktrace on default
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

#if !defined( __CYGWIN__) && !defined(__HAIKU__)
	printf("\n*********** BACKTRACK **************\n");

	count = backtrace(stack, 20);
	functions = backtrace_symbols(stack, count);
	sprintf(wholeStuff,"%s\n at line %d, file %s",info,lineno,file);
    string guiOut(wholeStuff); 
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
        guiOut+=string(buffer)+string("\n");
		
	}
	printf("*********** BACKTRACK **************\n");

	if(myFatalFunction)
		myFatalFunction("Crash", guiOut.c_str()); // FIXME
#endif	//__CYGWIN__

	exit(-1); // _exit(1) ???
}
#endif
//EOF
