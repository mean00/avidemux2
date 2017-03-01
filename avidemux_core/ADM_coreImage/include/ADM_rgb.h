/***************************************************************************
                         ADM_Rgb : wrapper for yv12->RGB display
                            using mplayer postproc


    begin                : Thu Apr 16 2003
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
 #ifndef ADM_RGB_H
 #define ADM_RGB_H

#define ADM_COLOR_IS_YUV 0x1000
 typedef enum 
 {
    ADM_COLOR_RGB24,
    ADM_COLOR_BGR24,
    ADM_COLOR_BGR32A,
    ADM_COLOR_RGB32A,
    ADM_COLOR_RGB16,
    ADM_COLOR_RGB555,
    ADM_COLOR_BGR555,
    ADM_COLOR_YV12 = ADM_COLOR_IS_YUV,	// YUV420
    ADM_COLOR_YUV422,
    ADM_COLOR_YUV422P,
    ADM_COLOR_YUV411,
    ADM_COLOR_YUV444,
    ADM_COLOR_VDPAU,
    ADM_COLOR_XVBA,
    ADM_COLOR_LIBVA,
    ADM_COLOR_YV12_10BITS,
    ADM_COLOR_Y8,
    ADM_COLOR_YUV444_10BITS,
    ADM_COLOR_NV12_10BITS
 } ADM_colorspace;
#define ADM_COLOR_BACKWARD 0x8000
#define ADM_COLOR_MASK     0x7FFF
 
 #endif
