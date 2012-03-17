/**
    \file ADM_jsAvidemux
    \brief Standard includes and defines
*/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef ADM_SCRIPT_COMMON
#define ADM_SCRIPT_COMMON
#include "ADM_inttype.h"
#ifdef __cplusplus
extern "C" {
#endif


//
int scriptLoadVideo(const char *c);
int scriptAppendVideo(const char *c);
int scriptClearSegments(void);
int scriptAddSegment(int ref, double start, double duration);
//-----------------------------------------
//-------------Audio-----------------------
//-----------------------------------------
int scriptAudioReset(int i);
int scriptAudioMixer(int i,const char *s);
// Normalize
int scriptGetNormalizeMode(int i);
int scriptGetNormalizeValue(int i);
int scriptSetNormalizeMode(int i,int v);
int scriptSetNormalizeValue(int i,int v);

// Fq
int32_t scriptGetResample(int i);
int    scriptSetResample(int i,int32_t fq);
//  Frame rate converter
int32_t scriptGetPal2film(int i);
int    scriptSetPal2film(int i,int32_t rate);
int32_t scriptGetFilm2pal(int i);
int    scriptSetFilm2pal(int i,int32_t rate);
// Markers
double scriptGetMarkerA(void);
double scriptGetMarkerB(void);
void   scriptSetMarkerA(double a);
void   scriptSetMarkerB(double b);
//
int    scriptClearVideoFilters();
// Info
int scriptGetWidth ( void) ;
int scriptGetHeight ( void) ;
int scriptGetPARWidth(void);
int scriptGetPARHeight(void);
int scriptGetFps1000 ( void) ;
char *jsGetVideoCodec ( void) ;
// Misc
int scriptSetPostProc (int a,int b, int c);
char *scriptGetVideoCodec ( void);

int  scriptGetNbSegment(void);
void scriptDumpSegment(int i);
#ifdef __cplusplus
};
#endif 
#endif
