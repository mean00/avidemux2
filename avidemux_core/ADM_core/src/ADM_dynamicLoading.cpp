/** *************************************************************************
    \fn 	ADM_dynamicLoading.cpp
    \brief 	Wrapper for dlopen & friends  
                      
    copyright            : (C) 2008 by Gruntster
    
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

#include "ADM_default.h"
#include "ADM_dynamicLoading.h"
#ifdef _WIN32
extern int utf8StringToWideChar(const char *utf8String, int utf8StringLength, wchar_t *wideCharString);
#endif

// By default the library is silent, being part of ADM_core cannot use the debug_id funcs
#if 1
#define aprintf(...) {}
#else
#define aprintf printf
#endif

ADM_LibWrapper::ADM_LibWrapper()
{
	initialised = false;
	hinstLib = NULL;
}

ADM_LibWrapper::~ADM_LibWrapper()
{
	if (hinstLib != NULL)
	{
		aprintf("Unloading library 0x%08x\n", hinstLib);

	#ifdef _WIN32
		FreeLibrary((HINSTANCE) hinstLib);
	#else
		dlclose(hinstLib);
	#endif
	}
}

bool ADM_LibWrapper::isAvailable()
{
	return initialised;
}

#ifdef _WIN32
char* ADM_LibWrapper::formatMessage(uint32_t msgCode)
{
	char* lpMsgBuf;

	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, msgCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &lpMsgBuf, 0, NULL);

	return lpMsgBuf;
}
#endif

bool ADM_LibWrapper::loadLibrary(const char* path)
{
#ifdef _WIN32
	int pathLength = utf8StringToWideChar(path, -1, NULL);
	wchar_t wcPath[pathLength];

	utf8StringToWideChar(path, -1, wcPath);

	hinstLib = LoadLibraryW(wcPath);

	if (hinstLib == NULL)
	{
		char* lpMsg = formatMessage(GetLastError());

		aprintf("Unable to load [%s]: %s\n", path, lpMsg);
		LocalFree(lpMsg);

		return false;
	}
	else
	{
		aprintf("Loaded library %s, handle = 0x%08x\n", path, hinstLib);

		return true;
	}
#else
	hinstLib = dlopen(path, RTLD_NOW | RTLD_LOCAL);
	
	if (hinstLib == NULL)
	{
		printf("Unable to load [%s]: %s\n", path, dlerror());

		return false;
	}
	else
	{
		aprintf("Loaded library %s, handle = 0x%08x\n", path, hinstLib);

		return true;
	}
#endif
}

void* ADM_LibWrapper::getSymbol(const char* name)
{
#ifdef _WIN32
	void* procAddr = (void*)GetProcAddress((HINSTANCE) hinstLib, name);

	if (procAddr == NULL)
	{
		char* lpMsg = formatMessage(GetLastError());

		aprintf("Unable to find symbol [%s]: %s\n", name, lpMsg);
		LocalFree(lpMsg);
	}

	return procAddr;
#else
	void* procAddr = dlsym(hinstLib, name);

	if (procAddr == NULL)
	{
		aprintf("Unable to find symbol [%s]: %s\n", name, dlerror());
	}

	return procAddr;
#endif
}

bool ADM_LibWrapper::getSymbols(int symCount, ...)
{
    va_list va;
    va_start(va, symCount);

    void** procFunction;
    char* funcName;
    int idxCount = 0;

    while (idxCount < symCount)
    {
        procFunction = va_arg(va, void**);
        funcName = va_arg(va, char*);

        if ((*procFunction = getSymbol(funcName)) == NULL)
        {
            procFunction = NULL;
            printf("[DynaLoader] Cannot find function %s\n",funcName);
            return false;
        }

        idxCount++;
    }

    va_end(va);

    return true;
}
