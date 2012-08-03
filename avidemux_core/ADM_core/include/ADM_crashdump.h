#ifndef ADM_CRASHDUMP_H
#define ADM_CRASHDUMP_H

#include "ADM_crashdump_mswin.h"
#include "ADM_crashdump_apple.h"
#include "ADM_crashdump_unix.h"

#ifdef __cplusplus
extern "C"
{
#endif

	typedef void ADM_saveFunction(void);
	typedef void ADM_fatalFunction(const char *title, const char *info);

	void ADM_backTrack(const char *info, int lineno, const char *file);
	void ADM_setCrashHook(ADM_saveFunction *save, ADM_fatalFunction *fatal);

#ifdef __cplusplus
}
#endif

#endif
