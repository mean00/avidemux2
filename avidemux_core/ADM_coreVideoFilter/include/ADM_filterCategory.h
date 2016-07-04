/***************************************************************************
                          video_filters.h  -  description
                             -------------------
    begin                : Wed Mar 27 2002
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
 #ifndef __VIDEO_FILTERS__CATEGORY__
 #define  __VIDEO_FILTERS__CATEGORY__

typedef enum
{
	VF_TRANSFORM=0,
	VF_INTERLACING=1,
	VF_COLORS=2,
	VF_NOISE=3,
	VF_SHARPNESS=4,
	VF_SUBTITLE=5,
        VF_OPENGL=6,
	VF_MISC=7,
        VF_HIDDEN=8,
	VF_MAX=9
}VF_CATEGORY;
#define VF_INVALID 		  0

#define VF_PARTIAL_FILTER 9999
#define VF_START_TAG 	  10

typedef uint32_t VF_FILTERS ;

 #endif
