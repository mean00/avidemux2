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
#include "ADM_confCouple.h"
#ifdef __cplusplus
extern "C" {
#endif


int     jsSetPostProc (int a,int b, int c);
int     scriptSetVideoCodec(const char *codec,CONFcouple *c);
int     scriptAddVideoFilter(const char *filter,CONFcouple *c);

#ifdef __cplusplus
};
#endif

#endif
