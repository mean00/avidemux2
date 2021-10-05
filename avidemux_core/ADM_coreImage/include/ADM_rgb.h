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

#define ADM_PIXFRMT_IS_YUV 0x1000
 typedef enum 
 {
    ADM_PIXFRMT_RGB24,
    ADM_PIXFRMT_BGR24,
    ADM_PIXFRMT_GBR24P,
    ADM_PIXFRMT_BGR32A,
    ADM_PIXFRMT_RGB32A,
    ADM_PIXFRMT_RGB16,
    ADM_PIXFRMT_RGB555,
    ADM_PIXFRMT_BGR555,
    ADM_PIXFRMT_YV12 = ADM_PIXFRMT_IS_YUV,	// YUV420
    ADM_PIXFRMT_NV12,
    ADM_PIXFRMT_YUV422,
    ADM_PIXFRMT_UYVY422,
    ADM_PIXFRMT_YUV422P,
    ADM_PIXFRMT_YUV411,
    ADM_PIXFRMT_YUV444,
    ADM_PIXFRMT_VDPAU,
    ADM_PIXFRMT_XVBA,
    ADM_PIXFRMT_LIBVA,
    ADM_PIXFRMT_Y8,
    ADM_PIXFRMT_YUV444_10BITS,
    ADM_PIXFRMT_NV12_10BITS,
    ADM_PIXFRMT_YUV420_10BITS,
    ADM_PIXFRMT_YUV420_12BITS,
    ADM_PIXFRMT_YUV422_10BITS,
    ADM_PIXFRMT_YUV444_12BITS
 } ADM_pixelFormat;
#define ADM_PIXFRMT_BACKWARD 0x8000
#define ADM_PIXFRMT_MASK     0x7FFF
 
 #endif
