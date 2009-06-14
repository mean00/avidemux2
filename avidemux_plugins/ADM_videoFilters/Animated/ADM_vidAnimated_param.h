/***************************************************************************
    copyright            : (C) 2006 by mean
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef ADM_VID_ANIMATED_PARAM_H
#define ADM_VID_ANIMATED_PARAM_H
#define MAX_VIGNETTE 6
typedef struct ANIMATED_PARAM
{
    uint32_t        vignetteW;
    uint32_t        vignetteH;
    uint32_t        isNTSC;
    uint32_t        timecode[MAX_VIGNETTE];
    ADM_filename    *backgroundImg;
}ANIMATED_PARAM;

#endif
