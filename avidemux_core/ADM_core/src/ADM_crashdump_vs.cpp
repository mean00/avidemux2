#include <windows.h>
#include <excpt.h>
#include <string>
#include "ADM_crashdump.h"
static ADM_saveFunction *mysaveFunction = NULL;
static ADM_fatalFunction *myFatalFunction = NULL;

void ADM_setCrashHook(ADM_saveFunction *save, ADM_fatalFunction *fatal,ADM_sigIntFunction *other) 
{
	mysaveFunction = save;
	myFatalFunction = fatal;
 }

/**
 * 
 * @param exceptionRecord
 * @param contextRecord
 */
void HandleException(const char *message,struct _EXCEPTION_RECORD *exceptionRecord, struct _CONTEXT *contextRecord)
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

	fflush(stdout);	
        
        const char *title;
        if(!message) title="Crash";
            else     title=message;
        if (myFatalFunction)
		myFatalFunction(title, s.c_str());
	    fflush(stderr);
	fflush(stdout);	
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
	HandleException("ExceptionHandler",exceptionRecord, contextRecord);
    return ExceptionContinueExecution;
}
/**
 * 
 * @param exceptionInfo
 * @return 
 */
LONG WINAPI ExceptionFilter(struct _EXCEPTION_POINTERS *exceptionInfo)
{
	HandleException("ExceptionFilter",exceptionInfo->ExceptionRecord, exceptionInfo->ContextRecord);
        return EXCEPTION_CONTINUE_SEARCH;
}
/**
 * 
 * @param pExceptionInfo
 * @return 
 */
LONG WINAPI TopLevelExceptionHandler(struct _EXCEPTION_POINTERS *exceptionInfo)
{
    HandleException("TopLevelExceptionHandler",exceptionInfo->ExceptionRecord, exceptionInfo->ContextRecord);
    return EXCEPTION_CONTINUE_SEARCH;
}

/**
 * 
 * @param info
 * @param lineno
 * @param file
 */
void ADM_backTrack(const char *info, int lineno, const char *file)
{	
    char title[2048]={0};
        snprintf(title,2000,"%s at line %d, file %s\n",info,lineno,file);
	HandleException(title,NULL, NULL);
}


/**
 * \fn installSigHandler
 * \brief add hook to catch exception (null pointers etc...)
 */
ADM_CORE6_EXPORT void installSigHandler(void)
{    
    SetUnhandledExceptionFilter(ExceptionFilter);
}
/**
 * \fn uninstallSigHandler
 * \brief
 */
ADM_CORE6_EXPORT void uninstallSigHandler(void)
{
    
}

