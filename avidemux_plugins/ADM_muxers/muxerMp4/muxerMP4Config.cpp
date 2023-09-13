/***************************************************************************
    copyright            : (C) 2007 by mean
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

#include "ADM_default.h"
#include "ADM_muxerInternal.h"
#include "muxerMP4.h"
#define ADM_MINIMAL_UI_INTERFACE
#include "DIA_factory.h"

#ifndef MUXER_IS_MOV
bool mp4Configure(void)
#else
bool movConfigure(void)
#endif
{
    mp4_muxer *cfg = &muxerConfig;

#define PX(x) &cfg->x
#define NB_ELEM(x) (sizeof(x)/sizeof(diaMenuEntry))

#ifndef MUXER_IS_MOV
    diaMenuEntry format[] = {
        {MP4_MUXER_MP4, "MP4", NULL},
        {MP4_MUXER_PSP, "PSP", NULL}
    };
    diaElemMenu menuFormat(PX(muxerType), QT_TRANSLATE_NOOP("mp4muxer","Muxing Format"), NB_ELEM(format), format, NULL);
#endif

    diaMenuEntry streamOpt[] = {
        {MP4_MUXER_OPT_NONE,        QT_TRANSLATE_NOOP("mp4muxer","No optimization"), NULL},
        {MP4_MUXER_OPT_FASTSTART,   QT_TRANSLATE_NOOP("mp4muxer","Move index to the beginning of the file"), NULL}
#ifndef MUXER_IS_MOV
       ,{MP4_MUXER_OPT_FRAGMENT,    QT_TRANSLATE_NOOP("mp4muxer","Use fragmentation"), NULL}
#endif
    };

    diaElemMenu menuOptimize(PX(optimize), QT_TRANSLATE_NOOP("mp4muxer","Optimize for Streaming"), NB_ELEM(streamOpt), streamOpt, NULL);
    diaElemToggle forceAR(PX(forceAspectRatio), QT_TRANSLATE_NOOP("mp4muxer","Force aspect ratio"));

    diaMenuEntry aspect[] = {
        {STANDARD,"4:3", NULL},
        {WIDE, "16:9", NULL},
        {UNI, "18:9", NULL},
        {CINEMA, "64:27", NULL},
        {CUSTOM, QT_TRANSLATE_NOOP("mp4muxer","Derived from display width"), NULL}
    };
    diaElemMenu menuAspect(PX(aspectRatio), QT_TRANSLATE_NOOP("mp4muxer","Aspect Ratio (DAR)"), NB_ELEM(aspect), aspect, NULL);
    diaElemUInteger dWidth(PX(displayWidth), QT_TRANSLATE_NOOP("mp4muxer","Display Width"), 16, 65535);

    forceAR.link(1,&menuAspect);
    menuAspect.link(aspect+4,1,&dWidth);

    diaMenuEntry rotation[] = {
        {MP4_MUXER_ROTATE_0,    QT_TRANSLATE_NOOP("mp4muxer","Do not rotate"), NULL},
        {MP4_MUXER_ROTATE_90,   QT_TRANSLATE_NOOP("mp4muxer","90°"), NULL},
        {MP4_MUXER_ROTATE_180,  QT_TRANSLATE_NOOP("mp4muxer","180°"), NULL},
        {MP4_MUXER_ROTATE_270,  QT_TRANSLATE_NOOP("mp4muxer","270°"), NULL}
    };
    diaElemMenu menuRotation(PX(rotation), QT_TRANSLATE_NOOP("mp4muxer","Rotate video"), NB_ELEM(rotation), rotation, NULL);

    diaMenuEntry clockFrequencies[] = {
        {MP4_MUXER_CLOCK_FREQ_AUTO,     QT_TRANSLATE_NOOP("mp4muxer","Auto"), NULL},
        {MP4_MUXER_CLOCK_FREQ_24KHZ,    QT_TRANSLATE_NOOP("mp4muxer","24 kHz"), NULL},
        {MP4_MUXER_CLOCK_FREQ_25KHZ,    QT_TRANSLATE_NOOP("mp4muxer","25 kHz"), NULL},
        {MP4_MUXER_CLOCK_FREQ_30KHZ,    QT_TRANSLATE_NOOP("mp4muxer","30 kHz"), NULL},
        {MP4_MUXER_CLOCK_FREQ_50KHZ,    QT_TRANSLATE_NOOP("mp4muxer","50 kHz"), NULL},
        {MP4_MUXER_CLOCK_FREQ_60KHZ,    QT_TRANSLATE_NOOP("mp4muxer","60 kHz"), NULL},
        {MP4_MUXER_CLOCK_FREQ_90KHZ,    QT_TRANSLATE_NOOP("mp4muxer","90 kHz"), NULL},
        {MP4_MUXER_CLOCK_FREQ_180KHZ,   QT_TRANSLATE_NOOP("mp4muxer","180 kHz"), NULL}
    };
    diaElemMenu menuClock(PX(clockfreq), QT_TRANSLATE_NOOP("mp4muxer","Time scale"), NB_ELEM(clockFrequencies), clockFrequencies, NULL);

    diaElem *tabs[] = {
#ifndef MUXER_IS_MOV
        &menuFormat,
#endif
        &menuOptimize,
        &forceAR,
        &menuAspect,
        &dWidth,
        &menuRotation,
        &menuClock
    };
    const char *title =
#ifndef MUXER_IS_MOV
        QT_TRANSLATE_NOOP("mp4muxer","MP4 Muxer");
#else
        QT_TRANSLATE_NOOP("mp4muxer","MOV Muxer");
#endif
#undef NB_ELEM
#define NB_ELEM(x) (sizeof(x)/sizeof(diaElem *))

    if(diaFactoryRun(title, NB_ELEM(tabs), tabs))
        return true;

    return false;
}

// EOF
