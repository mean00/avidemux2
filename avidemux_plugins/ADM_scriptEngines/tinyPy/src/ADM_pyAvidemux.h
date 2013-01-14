/***************************************************************************
   \file ADM_pyAvidemux.cpp
    \brief binding between tinyPy and avidemux
    \author mean/gruntster 2011/2012
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef ADM_PYAVIDEMUX_H
#define ADM_PYAVIDEMUX_H

#include "IEditor.h"

/* Audio */
int pyGetAudioChannels(IEditor *editor,int dex);
int pyGetAudioFrequency(IEditor *editor,int dex);
int pyGetAudioEncoding(IEditor *editor,int dex);
int pyAddAudioTrack(IEditor *editor, int poolindex);
int pyAddExternal(IEditor *editor, const char *fileName);
int pyClearAudioTracks(IEditor *editor);
int pyGetNumberOfAudioTracks(IEditor *editor);
/* Audio filters */
int pySetAudioShift(IEditor *editor,int track, int onoff, int value);
int pyGetAudioShift(IEditor *editor,int track, int *onoff, int *value);
int pyGetDrc(IEditor *editor,int track);
int pySetDrc(IEditor *editor,int track, int onoff);
int pyGetResample(IEditor *editor,int track);
int pySetResample(IEditor *editor,int track,int fq);
int32_t pyGetPal2Film(IEditor *editor);
int32_t pyGetFilm2Pal(IEditor *editor);
void pySetPal2Film(IEditor *editor, int32_t rate);
void pySetFilm2Pal(IEditor *editor, int32_t rate);
int pyGetNormalizeMode(IEditor *editor);
int pyGetNormalizeValue(IEditor *editor);
void pySetNormalizeMode(IEditor *editor, int mode);
void pySetNormalizeValue(IEditor *editor, int value);
int pySetFilm2Pal(IEditor *,int track,int onoff);
int pySetPal2Film(IEditor *,int track,int onoff);
int pySetNormalize(IEditor *,int track,int mode,int gain100);
/* Info */
int pyGetFps1000(IEditor *editor);
int pyGetWidth(IEditor *editor);
int pyGetHeight(IEditor *editor);

/* Detail info (debug) */
bool pyHexDumpFrame(IEditor *editor, int framenumber);
int pyPrintTiming(IEditor *editor, int framenumber);
double pyGetPts(IEditor *editor, int frameNum);
double pyGetDts(IEditor *editor, int frameNum);

/* File operation */
char *pyFileSelWrite(IEditor *editor, const char *title);
char *pyFileSelRead(IEditor *editor, const char *title);
char *pyDirSelect(IEditor *editor, const char *title);

/* Display */
void pyDisplayError(IEditor *editor, const char *one, const char *two);
void pyDisplayInfo(IEditor *editor, const char *one, const char *two);

/* Misc */
int pyTestCrash(void);
int pyTestAssert(void);

#endif
// EOF
