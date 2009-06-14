/***************************************************************************
                          Port of avisynth ColorYuv Filter
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

#ifndef ADM_VID_COLOR_YUV_PARAM_H
#define ADM_VID_COLOR_YUV_PARAM_H
typedef struct COLOR_YUV_PARAM
{
    double y_contrast, y_bright, y_gamma, y_gain;
    double u_contrast, u_bright, u_gamma, u_gain;
    double v_contrast, v_bright, v_gamma, v_gain;
    int32_t matrix, levels, opt;
    uint32_t colorbars, analyze, autowhite, autogain;

}COLOR_YUV_PARAM;

/*
matrix : 0 : none   1: rec.709
Level :  0 : none   1:"TV->PC", 2:"PC->TV", 3:"PC->TV.Y" 
Opt :    0 : none   1: Coring



*/

#endif
