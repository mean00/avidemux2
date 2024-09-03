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



#define ADM_INFO_COLOR      ""
#define ADM_WARNING_COLOR   "\e[31m\e[103m"
#define ADM_ERROR_COLOR     "\e[97m\e[41m"
#define ADM_FUNC_COLOR      "\e[36m"
#define ADM_RESET_COLOR     "\e[m"

#define ADM_INFO_MARK       "   "
#define ADM_WARNING_MARK    "<!>"
#define ADM_ERROR_MARK      "###"

extern "C"
{

static void ADM_prettyPrint(const char *func,const char *color, const char * mark, const char *p)
{
  // construct time code
  struct timeval pz;
  TIMZ tz;
  gettimeofday(&pz, &tz);
  long int tvSec=pz.tv_sec;
  long int tvUSec=pz.tv_usec;

  long int mseconds = tvUSec/1000;
  long int seconds=(tvSec)%60;
  long int mn=((tvSec)/60)%60;
  long int hh=((tvSec)/3600)%24;

#if _WIN32
      printf("%s%02d:%02d:%02d-%03d [%s] %s", mark,(int)hh,(int)mn,(int)seconds,(int)mseconds, func, p);
#else
    if(isatty(STDOUT_FILENO))
        printf("%s%02d:%02d:%02d-%03d%s [%s%s%s] %s", color,(int)hh,(int)mn,(int)seconds,(int)mseconds,ADM_RESET_COLOR, ADM_FUNC_COLOR,func,ADM_RESET_COLOR, p);
    else
        printf("%s%02d:%02d:%02d-%03d [%s] %s", mark,(int)hh,(int)mn,(int)seconds,(int)mseconds, func, p);
#endif
}


 void ADM_info2(const char *func, const char *prf, ...)
  {
  static char print_buffer[1024];

        va_list     list;
        va_start(list,    prf);
        vsnprintf(print_buffer,1023,prf,list);
        va_end(list);
        print_buffer[1023]=0; // ensure the string is terminated
        ADM_prettyPrint(func,ADM_INFO_COLOR,ADM_INFO_MARK,print_buffer);

  }
 void ADM_warning2( const char *func, const char *prf, ...)
  {
  static char print_buffer[1024];

        va_list     list;
        va_start(list,    prf);
        vsnprintf(print_buffer,1023,prf,list);
        va_end(list);
        print_buffer[1023]=0; // ensure the string is terminated
        ADM_prettyPrint(func,ADM_WARNING_COLOR,ADM_WARNING_MARK,print_buffer);

  }
 void ADM_error2( const char *func, const char *prf, ...)
  {
  static char print_buffer[1024];

        va_list     list;
        va_start(list,    prf);
        vsnprintf(print_buffer,1023,prf,list);
        va_end(list);
        print_buffer[1023]=0; // ensure the string is terminated
        ADM_prettyPrint(func,ADM_ERROR_COLOR,ADM_ERROR_MARK,print_buffer);

  }


}
//EOF
