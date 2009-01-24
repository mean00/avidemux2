/***************************************************************************
                          GUI_mux.h  -  description
                             -------------------
    begin                : Wed Jul 3 2002
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

 #ifndef __MUX_OPTIONS__
 #define __MUX_OPTIONS__

 typedef enum
 {
    	MUX_REGULAR=1,
     	MUX_N_FRAMES,
       MUX_N_BYTES
  } PARAM_MUX;

uint8_t  DIA_setUserMuxParam( int *mode, int *param,int *size);

#endif
