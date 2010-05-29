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

int scriptLoadVideo(const char *c);
int scriptAppendVideo(const char *c);
int scriptClearSegments(void);
int scriptAddSegment(int ref, double start, double duration);
int scriptAudioReset(void);
int scriptAudioMixer(const char *s);
// Fq
int32_t scriptGetResample(void);
void    scriptSetResample(int32_t fq);
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
int scriptGetFps1000 ( void) ;
char *jsGetVideoCodec ( void) ;
// Misc
int scriptSetPostProc (int a,int b, int c);
char *scriptGetVideoCodec ( void);

#ifdef __cplusplus
};
#endif 
#endif
