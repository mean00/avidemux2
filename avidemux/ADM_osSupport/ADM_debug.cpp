/***************************************************************************
                    Dully function to redirect unwanted printf


    begin                : Fri Apr 20 2003
    copyright            : (C) 2003 by mean
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

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "ADM_default.h"
#include "ADM_assert.h"
#include <math.h>


#include "ADM_osSupport/ADM_debugID.h"
#define MODULE_NAME MODULE_DEBUG
#include "ADM_debug.h"

// put here the module you want to be verbose (MODULE_xxx + MODULE_yyyy+  ....)
#ifndef masked
#define masked  (MODULE_3GP*0+ 1*MODULE_FILTER) //+MODULE_3GP) //(MODULE_AUDIO_EDITOR) MODULE_OGM_AUDIO MODULE_REQUANT MODULE_CODEC
#endif

#ifndef thresholdLevel
#define thresholdLevel ADM_PRINT_DEBUG
#endif

// If the entitty is in masked we actually print the string
// else we silently drop it
extern "C"
{
 void indirect_printf(int entity, const char *prf, ...)
  {
  static char print_buffer[1024];
  	if(masked & entity)
	{
		va_list 	list;
		va_start(list,	prf);
		vsnprintf(print_buffer,1023,prf,list);
		va_end(list);
		print_buffer[1023]=0; // ensure the string is terminated
		printf("%s",print_buffer);

	}
	else
  	{
		return;
	}
  }


void indirect_printf_long(int level,const char *modname,int entity,const char *prf,  ...)
{
  static char print_buffer[1024];
        if(level>thresholdLevel) return;
  	if(1) //masked & entity)
	{
		va_list 	list;
		va_start(list,	prf);
		vsnprintf(print_buffer,1023,prf,list);
		va_end(list);
		print_buffer[1023]=0; // ensure the string is terminated
		printf("[%s] LVL:%d %s",modname,level,print_buffer);

	}
	else
  	{
		return;
	}
}
}
//EOF
