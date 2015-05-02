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
#include <string>
#include <imagehlp.h>
#include <cxxabi.h>

static ADM_saveFunction *mysaveFunction = NULL;
static ADM_fatalFunction *myFatalFunction = NULL;
/**
 * 
 * @param save
 * @param fatal
 */
void ADM_setCrashHook(ADM_saveFunction *save, ADM_fatalFunction *fatal)
{
	mysaveFunction = save;
	myFatalFunction = fatal;
}
std::string shortModuleName(const char *module)
{
    std::string s=std::string(module);    
    std::size_t last=s.find_last_of('\\');
    if(last==std::string::npos)
        return s;
    return s.substr(last+1);
}
/**
 * 
 * @param moduleName
 * @param functionName
 * @param frameAddress
 * @param frameOffset
 * @return 
 */
static std::string PrintFunction(const char *moduleName, const char *functionName, DWORD_PTR frameAddress, DWORD_PTR frameOffset)
{
    std::string s;
    int status;

	if (functionName)
	{
            std::string cxaFunction=std::string("_");
                cxaFunction+=std::string(functionName);
		char *demangledName = __cxxabiv1::__cxa_demangle(cxaFunction.c_str(), NULL, NULL, &status);
		if (status == 0)
			s+=std::string(demangledName);
		else
			s+=std::string(functionName);

		if (demangledName)
			free(demangledName);
	}
	else
		s=std::string("unknown function");

//	if (frameOffset)
//		s+=std::string(" <+0x%X>", frameOffset);

	if (moduleName)
		s+=std::string("  [")+shortModuleName(moduleName)+std::string("] ");
	else
		s+=std::string("  [unknown module]");            
        s+=std::string("\n");
        return s;
}
/**
 * 
 * @param process
 * @param frameAddress
 * @return 
 */
static std::string  DumpFrame(void *process, DWORD_PTR frameAddress)
{
    std::string s;
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

	s+=PrintFunction(moduleName, functionName, frameAddress, displacement);

	free(symbol);
        return s;
}
/**
 * 
 * @param processId
 * @param exceptionRec
 * @param contextRecord
 * @return 
 */
static std::string DumpExceptionInfo(void *processId, struct _EXCEPTION_RECORD *exceptionRec, struct _CONTEXT *contextRecord)
{
    std::string s;
#ifdef _WIN64
	printf("RAX: %08X  RBX: %08X  RCX: %08X  RDX: %08X  RSI: %08X  RDI: %08X  RSP: %08X  RBP: %08X\n", contextRecord->Rax, contextRecord->Rbx, contextRecord->Rcx, contextRecord->Rdx, contextRecord->Rsi, contextRecord->Rdi, contextRecord->Rsp, contextRecord->Rbp);
	printf("R8: %08X  R9: %08X  R10: %08X  R11: %08X  R12: %08X  R13: %08X  R14: %08X  R15: %08X\n", contextRecord->R8, contextRecord->R9, contextRecord->R10, contextRecord->R11, contextRecord->R12, contextRecord->R13, contextRecord->R14, contextRecord->R15);
	printf("RIP: %08X  EFlags: %08X\n\n", contextRecord->Rip, contextRecord->EFlags);
#else
	//s+=std::string("EAX: %08X  EBX: %08X  ECX: %08X  EDX: %08X  ESI: %08X\n", contextRecord->Eax, contextRecord->Ebx, contextRecord->Ecx, contextRecord->Edx, contextRecord->Esi);
	//s+=std::string("EDI: %08X  ESP: %08X  EBP: %08X  EIP: %08X  EFlags: %08X\n\n", contextRecord->Edi, contextRecord->Esp, contextRecord->Ebp, contextRecord->Eip, contextRecord->EFlags);
#endif


	switch (exceptionRec->ExceptionCode)
	{
		case EXCEPTION_ACCESS_VIOLATION:
			s+=std::string("EXCEPTION_ACCESS_VIOLATION");
			break;
		case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
			s+=std::string("EXCEPTION_ARRAY_BOUNDS_EXCEEDED");
			break;
		case EXCEPTION_BREAKPOINT:
			s+=std::string("EXCEPTION_BREAKPOINT");
			break;
		case EXCEPTION_DATATYPE_MISALIGNMENT:
			s+=std::string("EXCEPTION_DATATYPE_MISALIGNMENT");
			break;
		case EXCEPTION_FLT_DENORMAL_OPERAND:
			s+=std::string("EXCEPTION_FLT_DENORMAL_OPERAND");
			break;
		case EXCEPTION_FLT_DIVIDE_BY_ZERO:
			s+=std::string("EXCEPTION_FLT_DIVIDE_BY_ZERO");
			break;
		case EXCEPTION_FLT_INEXACT_RESULT:
			s+=std::string("EXCEPTION_FLT_INEXACT_RESULT");
			break;
		case EXCEPTION_FLT_INVALID_OPERATION:
			s+=std::string("EXCEPTION_FLT_INVALID_OPERATION");
			break;
		case EXCEPTION_FLT_OVERFLOW:
			s+=std::string("EXCEPTION_FLT_OVERFLOW");
			break;
		case EXCEPTION_FLT_STACK_CHECK:
			s+=std::string("EXCEPTION_FLT_STACK_CHECK");
			break;
		case EXCEPTION_FLT_UNDERFLOW:
			printf("EXCEPTION_FLT_UNDERFLOW");
			break;
		case EXCEPTION_ILLEGAL_INSTRUCTION:
			s+=std::string("EXCEPTION_ILLEGAL_INSTRUCTION");
			break;
		case EXCEPTION_IN_PAGE_ERROR:
			s+=std::string("EXCEPTION_IN_PAGE_ERROR");
			break;
		case EXCEPTION_INT_DIVIDE_BY_ZERO:
			s+=std::string("EXCEPTION_INT_DIVIDE_BY_ZERO");
			break;
		case EXCEPTION_INT_OVERFLOW:
			s+=std::string("EXCEPTION_INT_OVERFLOW");
			break;
		case EXCEPTION_INVALID_DISPOSITION:
			s+=std::string("EXCEPTION_INVALID_DISPOSITION");
			break;
		case EXCEPTION_NONCONTINUABLE_EXCEPTION:
			s+=std::string("EXCEPTION_NONCONTINUABLE_EXCEPTION");
			break;
		case EXCEPTION_PRIV_INSTRUCTION:
			s+=std::string("EXCEPTION_PRIV_INSTRUCTION");
			break;
		case EXCEPTION_SINGLE_STEP:
			s+=std::string("EXCEPTION_SINGLE_STEP");
			break;
		case EXCEPTION_STACK_OVERFLOW:
			s+=std::string("EXCEPTION_STACK_OVERFLOW");
			break;
		default:
			s+=std::string("UNKNOWN");
	}
	s+=std::string("\n");
	//s+=std::string(" (%08X)\n", exceptionRec->ExceptionCode);
	//s+=std::string("Exception Flags: %08X\n", exceptionRec->ExceptionFlags);


#ifdef _WIN64
    #define REGISTER_RECORD Rip
#else
    #define REGISTER_RECORD Eip
#endif
	s+=DumpFrame(processId, contextRecord->REGISTER_RECORD);
        printf(s.c_str());
	fflush(stdout);
        return s;
}
/**
 * 
 * @param processId
 * @return 
 */
std::string DumpBackTrace(void *processId)
{
        std::string s;
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
            s+=DumpFrame(process, frame.AddrPC.Offset);
	}
        return s;
}
/**
 * 
 * @param exceptionRecord
 * @param contextRecord
 */
void HandleException(struct _EXCEPTION_RECORD *exceptionRecord, struct _CONTEXT *contextRecord)
{
    std::string s;
	fflush(stderr);
	fflush(stdout);	

	static int running = 0;

	if (running)
		exit(1);

	running = 1;

	if (mysaveFunction)
		mysaveFunction();

	

	void *process = GetCurrentProcess();

	SymInitialize(process, NULL, TRUE);

	if (exceptionRecord != NULL && contextRecord != NULL)
	{
		s+=DumpExceptionInfo(process, exceptionRecord, contextRecord);
	}

	s+=DumpBackTrace(process);
        if (myFatalFunction)
		myFatalFunction("Crash", s.c_str());
	SymCleanup(process);

	exit(1);
}
/**
 * 
 * @param exceptionRecord
 * @param establisherFrame
 * @param contextRecord
 * @param dispatcherContext
 * @return 
 */
EXCEPTION_DISPOSITION ExceptionHandler(struct _EXCEPTION_RECORD *exceptionRecord, void *establisherFrame, struct _CONTEXT *contextRecord, void *dispatcherContext)
{
	HandleException(exceptionRecord, contextRecord);
}
/**
 * 
 * @param exceptionInfo
 * @return 
 */
LONG WINAPI ExceptionFilter(struct _EXCEPTION_POINTERS *exceptionInfo)
{
	HandleException(exceptionInfo->ExceptionRecord, exceptionInfo->ContextRecord);
}
/**
 * 
 * @param info
 * @param lineno
 * @param file
 */
void ADM_backTrack(const char *info, int lineno, const char *file)
{	
	HandleException(NULL, NULL);
}
