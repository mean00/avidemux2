/***************************************************************************
                          ADM_lavcodec.h  -  description
                             -------------------
    begin                : Tue Nov 12 2002
    copyright            : (C) 2002 by mean
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
 #ifndef __ADM_LAVC
  #define  __ADM_LAVC
  extern "C"
  {  
  #define __STDC_CONSTANT_MACROS  1 // Lavcodec crap
  #define __STDC_LIMIT_MACROS 1
  #include "ADM_ffmpeg/libavcodec/avcodec.h"
  };
#endif
