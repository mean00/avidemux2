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

#include "ADM_crashdump.h"
#include <stdio.h>
#include <imagehlp.h>
#include <cxxabi.h>

extern "C"
{
typedef void ADM_saveFunction(void);
typedef void ADM_fatalFunction(const char *title, const char *info);
void ADM_setCrashHook(ADM_saveFunction *save, ADM_fatalFunction *fatal);
void ADM_backTrack(const char *info, int lineno, const char *file);
}

static ADM_saveFunction *mysaveFunction = NULL;
static ADM_fatalFunction *myFatalFunction = NULL;

void ADM_setCrashHook(ADM_saveFunction *save, ADM_fatalFunction *fatal)
{
	mysaveFunction = save;
	myFatalFunction = fatal;
}

static void PrintFunction(const char *moduleName, const char *functionName, DWORD_PTR frameAddress, DWORD_PTR frameOffset)
{
	if (functionName)
	{
		char *cxaFunction = (char*)malloc(strlen(functionName) + 2);

		cxaFunction[0] = '_';
		strcpy(cxaFunction + 1, functionName);

		int status;
		char *demangledName = __cxxabiv1::__cxa_demangle(cxaFunction, NULL, NULL, &status);

		free(cxaFunction);

		if (status == 0)
			printf(demangledName);
		else
			printf(functionName);

		if (demangledName)
			free(demangledName);
	}
	else
		printf("unknown function");

	if (frameOffset)
		printf(" <+0x%X>", frameOffset);

	if (moduleName)
		printf("  [%s]\n", moduleName);
	else
		printf("  [unknown module]\n");

	fflush(stdout);
}

static bool DumpFrame(void *process, DWORD_PTR frameAddress)
{
	const int functionLength = 255;
	IMAGEHLP_SYMBOL *symbol = (IMAGEHLP_SYMBOL*)malloc(sizeof(IMAGEHLP_SYMBOL) + functionLength);
	DWORD_PTR moduleBase = SymGetModuleBase(process, frameAddress);
	const char *moduleName = NULL, *functionName = NULL;
	DWORD_PTR displacement;
	char moduleFilename[MAX_PATH];

	symbol->SizeOfStruct = (sizeof(*symbol)) + functionLength;
	symbol->MaxNameLength = functionLength - 1;

	if (moduleBase && GetModuleFileName((HINSTANCE)moduleBase, moduleFilename, MAX_PATH))
		moduleName = moduleFilename;

	if (SymGetSymFromAddr(process, frameAddress, &displacement, symbol))
		functionName = symbol->Name;

	PrintFunction(moduleName, functionName, frameAddress, displacement);

	free(symbol);
}

static void DumpExceptionInfo(void *processId, struct _EXCEPTION_RECORD *exceptionRec, struct _CONTEXT *contextRecord)
{
	printf("\n*********** EXCEPTION **************\n");
	printf("Registers:\n");
#ifdef _WIN64
	printf("RAX: %08X  RBX: %08X  RCX: %08X  RDX: %08X  RSI: %08X  RDI: %08X  RSP: %08X  RBP: %08X\n", contextRecord->Rax, contextRecord->Rbx, contextRecord->Rcx, contextRecord->Rdx, contextRecord->Rsi, contextRecord->Rdi, contextRecord->Rsp, contextRecord->Rbp);
	printf("R8: %08X  R9: %08X  R10: %08X  R11: %08X  R12: %08X  R13: %08X  R14: %08X  R15: %08X\n", contextRecord->R8, contextRecord->R9, contextRecord->R10, contextRecord->R11, contextRecord->R12, contextRecord->R13, contextRecord->R14, contextRecord->R15);
	printf("RIP: %08X  EFlags: %08X\n\n", contextRecord->Rip, contextRecord->EFlags);
#else
	printf("EAX: %08X  EBX: %08X  ECX: %08X  EDX: %08X  ESI: %08X\n", contextRecord->Eax, contextRecord->Ebx, contextRecord->Ecx, contextRecord->Edx, contextRecord->Esi);
	printf("EDI: %08X  ESP: %08X  EBP: %08X  EIP: %08X  EFlags: %08X\n\n", contextRecord->Edi, contextRecord->Esp, contextRecord->Ebp, contextRecord->Eip, contextRecord->EFlags);
#endif

	printf("Exception Code: ");

	switch (exceptionRec->ExceptionCode)
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
	
	printf(" (%08X)\n", exceptionRec->ExceptionCode);
	printf("Exception Flags: %08X\n", exceptionRec->ExceptionFlags);

	printf("\nOrigin:\n");

#ifdef _WIN64
	DumpFrame(processId, contextRecord->Rip);
#else
	DumpFrame(processId, contextRecord->Eip);
#endif

	printf("*********** EXCEPTION **************\n");
	fflush(stdout);
}

void DumpBackTrace(void *processId)
{
	printf("\n*********** BACKTRACE **************\n");

	typedef VOID NTAPI RtlCaptureContext_(PCONTEXT ContextRecord);

	HANDLE process = GetCurrentProcess();
	HANDLE thread = GetCurrentThread();
	HINSTANCE hinstLib = LoadLibrary("kernel32.dll");
	RtlCaptureContext_* contextFunc = (RtlCaptureContext_*)GetProcAddress(hinstLib, "RtlCaptureContext");
	STACKFRAME frame;
	CONTEXT context;
	int limit = 50;
	DWORD machineType;

	memset(&frame, 0, sizeof(STACKFRAME));
	memset(&context, 0, sizeof(CONTEXT));

	context.ContextFlags = CONTEXT_FULL;
	contextFunc(&context);

#if _WIN64
	machineType = IMAGE_FILE_MACHINE_AMD64;

    frame.AddrPC.Offset = context.Rip;
	frame.AddrStack.Offset = context.Rsp;
	frame.AddrFrame.Offset = context.Rbp;
#else
	machineType = IMAGE_FILE_MACHINE_I386;

	frame.AddrPC.Offset = context.Eip;
	frame.AddrStack.Offset = context.Esp;
	frame.AddrFrame.Offset = context.Ebp;
#endif

    frame.AddrPC.Mode = AddrModeFlat;
    frame.AddrStack.Mode = AddrModeFlat;
    frame.AddrFrame.Mode = AddrModeFlat;

	while (StackWalk(machineType, process, thread, &frame, &context, 0, SymFunctionTableAccess, SymGetModuleBase, 0))
	{
        if (limit-- == 0)
			break;

		DumpFrame(process, frame.AddrPC.Offset);
	}

	printf("*********** BACKTRACE **************\n\n");
}

void HandleException(struct _EXCEPTION_RECORD *exceptionRecord, struct _CONTEXT *contextRecord)
{
	fflush(stderr);
	fflush(stdout);	

	static int running = 0;

	if (running)
		exit(1);

	running = 1;

	if (mysaveFunction)
		mysaveFunction();

	if (myFatalFunction)
		myFatalFunction("Crash", "Press OK to build crash info");

	void *process = GetCurrentProcess();

	SymInitialize(process, NULL, TRUE);

	if (exceptionRecord != NULL && contextRecord != NULL)
	{
		DumpExceptionInfo(process, exceptionRecord, contextRecord);
	}

	DumpBackTrace(process);
	SymCleanup(process);

	exit(1);
}

EXCEPTION_DISPOSITION ExceptionHandler(struct _EXCEPTION_RECORD *exceptionRecord, void *establisherFrame, struct _CONTEXT *contextRecord, void *dispatcherContext)
{
	HandleException(exceptionRecord, contextRecord);
}

LONG WINAPI ExceptionFilter(struct _EXCEPTION_POINTERS *exceptionInfo)
{
	HandleException(exceptionInfo->ExceptionRecord, exceptionInfo->ContextRecord);
}

void ADM_backTrack(const char *info, int lineno, const char *file)
{	
	HandleException(NULL, NULL);
}
