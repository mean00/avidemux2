/***************************************************************************
 strnew.h  -  description
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

#ifndef __STRNEW_H__
#define __STRNEW_H__
#ifdef __cplusplus
extern "C" char *strnew (const char *instr);
#else
extern char *strnew (const char *instr);
#endif
#endif // __STRNEW_H__

