/***************************************************************************
                    Dummy function to redirect unwanted printf


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

#include "ADM_default.h"
#include <stdarg.h>
#include <math.h>




#define ADM_COLOR_YELLOW  "\e[33m"
#define ADM_COLOR_RED     "\e[31m"
#define ADM_COLOR_GREEN   "\e[32m"
#define ADM_DEFAULT_COLOR "\e[m"
 
extern "C"
{

static void ADM_prettyPrint(const char *func,const char *color, const char *p)
{
#if _WIN32
	printf("[%s] %s", func, p);
#else
    printf("%s [%s]  %s %s",color,func,p,ADM_DEFAULT_COLOR);
#endif
}


 void ADM_info2(const char *func, const char *prf, ...)
  {
  static char print_buffer[1024];
  	
		va_list 	list;
		va_start(list,	prf);
		vsnprintf(print_buffer,1023,prf,list);
		va_end(list);
		print_buffer[1023]=0; // ensure the string is terminated
        ADM_prettyPrint(func,ADM_COLOR_GREEN,print_buffer);
		
  }
 void ADM_warning2( const char *func, const char *prf, ...)
  {
  static char print_buffer[1024];
  	
		va_list 	list;
		va_start(list,	prf);
		vsnprintf(print_buffer,1023,prf,list);
		va_end(list);
		print_buffer[1023]=0; // ensure the string is terminated
        ADM_prettyPrint(func,ADM_COLOR_YELLOW,print_buffer);
		
  }
 void ADM_error2( const char *func, const char *prf, ...)
  {
  static char print_buffer[1024];
  	
		va_list 	list;
		va_start(list,	prf);
		vsnprintf(print_buffer,1023,prf,list);
		va_end(list);
		print_buffer[1023]=0; // ensure the string is terminated
        ADM_prettyPrint(func,ADM_COLOR_RED,print_buffer);
		
  }


}
//EOF
