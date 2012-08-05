/** *************************************************************************
    \fn 	ADM_dynamicLoading.h
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

#ifndef ADM_DYNAMICLOADING_H
#define ADM_DYNAMICLOADING_H

#include "ADM_core6_export.h"

#ifdef _WIN32
#	include "ADM_inttype.h"

#	define SHARED_LIB_EXT "dll"
#elif defined(__APPLE__)
#	define SHARED_LIB_EXT "dylib"
#else
#	define SHARED_LIB_EXT "so"
#endif

class ADM_CORE6_EXPORT ADM_LibWrapper
{
	protected:
		void* hinstLib;
		bool initialised;

	#ifdef _WIN32
		virtual char* formatMessage(uint32_t msgCode);
	#endif

    public:
		ADM_LibWrapper();
		virtual ~ADM_LibWrapper();
		virtual bool loadLibrary(const char* path);
		virtual void* getSymbol(const char* name);
		virtual bool getSymbols(int symCount, ...);
		virtual bool isAvailable();
};

#endif
