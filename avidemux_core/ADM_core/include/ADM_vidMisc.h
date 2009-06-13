/***************************************************************************
                          ADM_vidMisc.h  -  description
    copyright            : (C) 2001 by mean
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

#ifndef __ADM_VIDMISC__
#define __ADM_VIDMISC__

void            frame2time(uint32_t frame, uint32_t fps, uint16_t * hh, uint16_t * mm,
                                uint16_t * ss, uint16_t * ms);
void            time2frame(uint32_t *frame, uint32_t fps, uint32_t hh, uint32_t mm,
                                uint32_t ss, uint32_t ms);
void            ms2time(uint32_t len2,uint16_t * hh, uint16_t * mm,
                                uint16_t * ss, uint16_t * ms);				
char*			ms2timedisplay(uint32_t ms);

#define FRAME_PAL 1
#define FRAME_FILM 2
#define FRAME_NTSC 3

uint8_t 	identMovieType(uint32_t fps1000); // identify the movie type (mainly for mpeg1/2) 
uint8_t 	ms2time(uint32_t ms, uint32_t *h,uint32_t *m, uint32_t *s);
#endif
