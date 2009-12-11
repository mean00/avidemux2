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
extern "C" {
#endif

int jsVideoCodec(const char *codec,const char **p);
int jsSetPostProc (int a,int b, int c);

#ifdef __cplusplus
};
#endif

#endif
