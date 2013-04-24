/***************************************************************************
 cdebug.h  -  description
 -------------------
 begin                : 28-04-2008
 copyright            : (C) 2008 by fahr
 email                : fahr at inbox dot ru
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef __CDEBUG__
#define __CDEBUG__
#ifdef DEBUGMSG
extern "C" void dbgprintf (const char *format, ...);
extern "C" void dbgprintf_RED (const char *format, ...);
extern "C" void setdbglog (const char *fname);
#define DEBUG_PRINTF dbgprintf
#define DEBUG_PRINTF_RED dbgprintf_RED
#else
#define DEBUG_PRINTF_RED
#define DEBUG_PRINTF
#endif

#endif
