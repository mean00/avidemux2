/**
    \file ADM_jsVideo
    \brief Video codec etc..
*/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef ADM_JS_VIDEO_H
#define ADM_JS_VIDEO_H
#ifdef __cplusplus
#include "ADM_confCouple.h"
extern "C" {
#else
#endif


int     scriptSetPostProc (int a,int b, int c);
#ifdef __cplusplus
int     scriptSetVideoCodec(const char *codec,CONFcouple *c); // Only take the full param list
int     scriptAddVideoFilter(const char *filter,CONFcouple *c);
int     scriptSetVideoCodecParam(const char *codec,CONFcouple *c); // same as setVideoCodec, but accepts partial list

#endif
#ifdef __cplusplus
};
#endif

#endif
