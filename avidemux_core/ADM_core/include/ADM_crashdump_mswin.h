#if defined(_WIN32) && !defined ADM_CRASHDUMP_MSWIN_H
#define ADM_CRASHDUMP_MSWIN_H

#if defined(_WIN64)
LONG WINAPI ExceptionFilter(struct _EXCEPTION_POINTERS *exceptionInfo);
#define installSigHandler() SetUnhandledExceptionFilter(ExceptionFilter)
#define uninstallSigHandler()
#else
EXCEPTION_DISPOSITION ExceptionHandler(struct _EXCEPTION_RECORD *exceptionRecord, void *establisherFrame, struct _CONTEXT *contextRecord, void *dispatcherContext);
#define installSigHandler() __try1(ExceptionHandler)
#define uninstallSigHandler() __except1
#endif

#endif
