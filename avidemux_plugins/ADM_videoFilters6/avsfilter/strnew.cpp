/***************************************************************************
 strnew.cpp  -  description
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

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef __MINGW32__
extern "C" void *ADM_alloc(int sz);
#else
#define ADM_alloc malloc
#endif

extern "C" char *strnew (const char *instr)
{
  if (!instr)
    return NULL;
  size_t sl = strlen (instr);
  char *tmp = (char*) ADM_alloc (sl + 1);
  memcpy (tmp, instr, sl + 1);
  return tmp;
}
