/***************************************************************************
                          adm_ogg.h  -  description
                             -------------------
    begin                : Thu May 23 2002
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

uint8_t 	    ADM_OggInit(void );
uint8_t 	    ADM_OggEnd(void );
uint8_t 		ADM_OggRun(uint8_t * ptr, uint32_t nbIn, uint8_t * outptr,   uint32_t * nbOut);
