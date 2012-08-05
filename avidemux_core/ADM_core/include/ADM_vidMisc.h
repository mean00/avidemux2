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

#include "ADM_core6_export.h"

ADM_CORE6_EXPORT void            frame2time(uint32_t frame, uint32_t fps, uint32_t * hh, uint32_t * mm, uint32_t * ss, uint32_t * ms);
ADM_CORE6_EXPORT void            time2frame(uint32_t *frame, uint32_t fps, uint32_t hh, uint32_t mm, uint32_t ss, uint32_t ms);
char*			ms2timedisplay(uint32_t ms);
ADM_CORE6_EXPORT uint8_t         ms2time(uint32_t ms, uint32_t *h,uint32_t *m, uint32_t *s,uint32_t *mms);

#define FRAME_PAL 1
#define FRAME_FILM 2
#define FRAME_NTSC 3

uint8_t 	identMovieType(uint32_t fps1000); // identify the movie type (mainly for mpeg1/2) 
ADM_CORE6_EXPORT const char *ADM_us2plain(uint64_t ams);
#endif
