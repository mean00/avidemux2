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
int pyGetAudioBitrate(IEditor *editor,int dex);
int pyGetAudioChannels(IEditor *editor,int dex);
int pyGetAudioFrequency(IEditor *editor,int dex);
int pyGetAudioEncoding(IEditor *editor,int dex);
int pyAddAudioTrack(IEditor *editor, int poolindex);
int pyAddExternal(IEditor *editor, const char *fileName);
int pyClearAudioTracks(IEditor *editor);
int pyGetNumberOfAudioTracks(IEditor *editor);
int pyGetNumberOfAvailableAudioTracks(IEditor *editor);
/* Audio filters */
int pySetAudioShift(IEditor *editor,int track, int onoff, int value);
int pyGetAudioShift(IEditor *editor,int track, int *onoff, int *value);
int pySetDrc(IEditor *editor,int track, int active);
int pyGetDrc2(IEditor *editor,int track, int * active, int * normalize, float * nFloor, float * attTime, float * decTime, float * ratio, float * thresDB);
int pySetDrc2(IEditor *editor,int track, int active, int normalize, float nFloor, float attTime, float decTime, float ratio, float thresDB);
int pyGetEq(IEditor *editor,int track, int * active, float * lo, float * md, float * hi, float * lmcut, float * mhcut);
int pySetEq(IEditor *editor,int track, int active, float lo, float md, float hi, float lmcut, float mhcut);
int pyGetFade(IEditor *editor,int track, float * fadeIn, float * fadeOut, int * videoFilterBridge);
int pySetFade(IEditor *editor,int track, float fadeIn, float fadeOut, int videoFilterBridge);
int pyGetChGains(IEditor *editor,int track, float * fL, float * fR, float * fC, float * sL, float * sR, float * rL, float * rR, float * rC, float * LFE);
int pySetChGains(IEditor *editor,int track, float fL, float fR, float fC, float sL, float sR, float rL, float rR, float rC, float LFE);
int pyGetChDelays(IEditor *editor,int track, int * fL, int * fR, int * fC, int * sL, int * sR, int * rL, int * rR, int * rC, int * LFE);
int pySetChDelays(IEditor *editor,int track, int fL, int fR, int fC, int sL, int sR, int rL, int rR, int rC, int LFE);
int pyGetChRemap(IEditor *editor,int track, int * active, int * fL, int * fR, int * fC, int * sL, int * sR, int * rL, int * rR, int * rC, int * LFE);
int pySetChRemap(IEditor *editor,int track, int active, int fL, int fR, int fC, int sL, int sR, int rL, int rR, int rC, int LFE);
int pyGetResample(IEditor *editor,int track);
int pySetResample(IEditor *editor,int track,int fq);
int32_t pyGetPal2Film(IEditor *editor);
int32_t pyGetFilm2Pal(IEditor *editor);
void pySetPal2Film(IEditor *editor, int32_t rate);
void pySetFilm2Pal(IEditor *editor, int32_t rate);
int pyGetNormalizeMode(IEditor *editor);
int pyGetNormalizeValue(IEditor *editor);
int pyGetNormalizeLevel(IEditor *editor);
void pySetNormalizeMode(IEditor *editor, int mode);
void pySetNormalizeValue(IEditor *editor, int value);
void pySetNormalizeLevel(IEditor *editor, int level);
int pySetNormalize(IEditor *, int track, int mode, int gain100);
int pySetNormalize2(IEditor *, int track, int mode, int gain100, int maxlevel);
int pySetFilm2Pal(IEditor *,int track,int onoff);
int pySetPal2Film(IEditor *,int track,int onoff);
int pySetCustomAudioFrameRate(IEditor *,int track,double tempo, double pitch);
/* Output */
char *pyGetContainerEx(IEditor *editor);
/* Info */
int pyGetFps1000(IEditor *editor);
int pyGetWidth(IEditor *editor);
int pyGetHeight(IEditor *editor);

/* Editing-related info */
int pyGetCurrentFrameFlags(IEditor *editor);
double pyGetPrevKFramePts(IEditor *editor, double time);
double pyGetNextKFramePts(IEditor *editor, double time);
int pySegmentGetRefIdx(IEditor *editor, int segment);
double pySegmentGetTimeOffset(IEditor *editor, int segment);
double pySegmentGetDuration(IEditor *editor, int segment);
double pyGetRefVideoDuration(IEditor *editor, int refVideoIdx);
char *pyGetRefVideoName(IEditor *editor, int idx);

/* Detail info (debug) */
int pyHexDumpFrame(IEditor *editor, int framenumber);
int pyPrintTiming(IEditor *editor, int framenumber);
int pyPrintFrameInfo(IEditor *editor, int framenumber);
double pyGetPts(IEditor *editor, int frameNum);
double pyGetDts(IEditor *editor, int frameNum);

/* File operation */
char *pyFileSelWrite(IEditor *editor, const char *title);
char *pyFileSelRead(IEditor *editor, const char *title);
char *pyFileSelWriteEx(IEditor *editor, const char *title, const char *ext);
char *pyFileSelReadEx(IEditor *editor, const char *title, const char *ext);
char *pyDirSelect(IEditor *editor, const char *title);

/* Display */
void pyDisplayError(IEditor *editor, const char *one, const char *two);
void pyDisplayInfo(IEditor *editor, const char *one, const char *two);

/* Misc */
int pyTestCrash(void);
int pyTestAssert(void);
int pyTestSub( char *subName);
/* OS */
char *pyGetEnv(IEditor *editor,const char *);
/* Navigate */
int pyNextFrame(IEditor *editor);
#endif
// EOF
