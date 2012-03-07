#ifndef ADM_PYAVIDEMUX_H
#define ADM_PYAVIDEMUX_H

#include "IEditor.h"

int pyChangeAudioStream(IEditor *editor, int track);
int pyGetFps1000(IEditor *editor);
int pyGetWidth(IEditor *editor);
int pyGetHeight(IEditor *editor);
int pyGetAudioChannels(IEditor *editor);
int pyGetAudioFrequency(IEditor *editor);
int pyGetAudioEncoding(IEditor *editor);
void pySetAudioFrequency(IEditor *editor, int fq);
void pySetAudioEncoding(IEditor *editor, int enc);
void pySetAudioChannels(IEditor *editor, int dq);
int32_t pyGetPal2Film(IEditor *editor);
int32_t pyGetFilm2Pal(IEditor *editor);
void pySetPal2Film(IEditor *editor, int32_t rate);
void pySetFilm2Pal(IEditor *editor, int32_t rate);
int pyGetNormalizeMode(IEditor *editor);
int pyGetNormalizeValue(IEditor *editor);
void pySetNormalizeMode(IEditor *editor, int mode);
void pySetNormalizeValue(IEditor *editor, int value);
bool pyHexDumpFrame(IEditor *editor, int framenumber);
int pyPrintTiming(IEditor *editor, int framenumber);
double pyGetPts(IEditor *editor, int frameNum);
double pyGetDts(IEditor *editor, int frameNum);
char *pyFileSelWrite(IEditor *editor, const char *title);
char *pyFileSelRead(IEditor *editor, const char *title);
char *pyDirSelect(IEditor *editor, const char *title);
void pyDisplayError(IEditor *editor, const char *one, const char *two);
void pyDisplayInfo(IEditor *editor, const char *one, const char *two);
int pyTestCrash(void);
int pyTestAssert(void);

#endif
