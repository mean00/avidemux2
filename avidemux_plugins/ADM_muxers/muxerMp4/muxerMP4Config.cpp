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
        uint32_t fmt = muxerConfig.muxerType;
        uint32_t dar = muxerConfig.aspectRatio;
        uint32_t diw = muxerConfig.displayWidth;
        uint32_t rot = muxerConfig.rotation;
        uint32_t opt = muxerConfig.optimize;
        uint32_t clk = muxerConfig.clockfreq;
        bool force   = muxerConfig.forceAspectRatio;

#ifndef MUXER_IS_MOV
        diaMenuEntry format[]={{MP4_MUXER_MP4,"MP4",NULL},{MP4_MUXER_PSP,"PSP",NULL}};
        diaElemMenu  menuFormat(&fmt,QT_TRANSLATE_NOOP("mp4muxer","Muxing Format"),2,format,"");
        diaMenuEntry streamOpt[]={
            {MP4_MUXER_OPT_NONE,QT_TRANSLATE_NOOP("mp4muxer","No optimization"),NULL},
            {MP4_MUXER_OPT_FASTSTART,QT_TRANSLATE_NOOP("mp4muxer","Move index to the beginning of the file"),NULL},
            {MP4_MUXER_OPT_FRAGMENT,QT_TRANSLATE_NOOP("mp4muxer","Use fragmentation"),NULL}
        };
#else
        diaMenuEntry streamOpt[]={
            {MP4_MUXER_OPT_NONE,QT_TRANSLATE_NOOP("mp4muxer","No optimization"),NULL},
            {MP4_MUXER_OPT_FASTSTART,QT_TRANSLATE_NOOP("mp4muxer","Move index to the beginning of the file"),NULL}
        };

#endif

#ifndef MUXER_IS_MOV
#   define NB_OPTIMIZE 3
#   define NB_TABS 7
#else
#   define NB_TABS 6
#   define NB_OPTIMIZE 2
#endif
        diaElemMenu  menuOptimize(&opt,QT_TRANSLATE_NOOP("mp4muxer","Optimize for Streaming"),NB_OPTIMIZE,streamOpt,"");
        diaElemToggle forceAR(&force,QT_TRANSLATE_NOOP("mp4muxer","Force aspect ratio"));
        diaMenuEntry aspect[]={
            {STANDARD,"4:3",NULL},
            {WIDE,"16:9",NULL},
            {UNI,"18:9",NULL},
            {CINEMA,"64:27",NULL},
            {CUSTOM,QT_TRANSLATE_NOOP("mp4muxer","Derived from display width"),NULL}
        };
        diaElemMenu  menuAspect(&dar,QT_TRANSLATE_NOOP("mp4muxer","Aspect Ratio (DAR)"),5,aspect,"");
        diaElemUInteger dWidth(&diw,QT_TRANSLATE_NOOP("mp4muxer","Display Width"),16,65535);
        forceAR.link(1,&menuAspect);
        menuAspect.link(aspect+4,1,&dWidth);
        diaMenuEntry rotation[]={
            {MP4_MUXER_ROTATE_0,QT_TRANSLATE_NOOP("mp4muxer","Do not rotate"),NULL},
            {MP4_MUXER_ROTATE_90,QT_TRANSLATE_NOOP("mp4muxer","90°"),NULL},
            {MP4_MUXER_ROTATE_180,QT_TRANSLATE_NOOP("mp4muxer","180°"),NULL},
            {MP4_MUXER_ROTATE_270,QT_TRANSLATE_NOOP("mp4muxer","270°"),NULL}
        };
        diaElemMenu menuRotation(&rot,QT_TRANSLATE_NOOP("mp4muxer","Rotate video"),4,rotation,"");

        diaMenuEntry clockFrequencies[]={
            {MP4_MUXER_CLOCK_FREQ_AUTO,QT_TRANSLATE_NOOP("mp4muxer","Auto"),NULL},
            {MP4_MUXER_CLOCK_FREQ_24KHZ,QT_TRANSLATE_NOOP("mp4muxer","24 kHz"),NULL},
            {MP4_MUXER_CLOCK_FREQ_25KHZ,QT_TRANSLATE_NOOP("mp4muxer","25 kHz"),NULL},
            {MP4_MUXER_CLOCK_FREQ_30KHZ,QT_TRANSLATE_NOOP("mp4muxer","30 kHz"),NULL},
            {MP4_MUXER_CLOCK_FREQ_50KHZ,QT_TRANSLATE_NOOP("mp4muxer","50 kHz"),NULL},
            {MP4_MUXER_CLOCK_FREQ_60KHZ,QT_TRANSLATE_NOOP("mp4muxer","60 kHz"),NULL},
            {MP4_MUXER_CLOCK_FREQ_90KHZ,QT_TRANSLATE_NOOP("mp4muxer","90 kHz"),NULL},
            {MP4_MUXER_CLOCK_FREQ_180KHZ,QT_TRANSLATE_NOOP("mp4muxer","180 kHz"),NULL}
        };
        diaElemMenu menuClock(&clk,QT_TRANSLATE_NOOP("mp4muxer","Time scale"),8,clockFrequencies,NULL);

        const char *title;
#ifndef MUXER_IS_MOV
        diaElem *tabs[]={&menuFormat,&menuOptimize,&forceAR,&menuAspect,&dWidth,&menuRotation,&menuClock};
        title = QT_TRANSLATE_NOOP("mp4muxer","MP4 Muxer");
#else
        diaElem *tabs[]={            &menuOptimize,&forceAR,&menuAspect,&dWidth,&menuRotation,&menuClock};
        title = QT_TRANSLATE_NOOP("mp4muxer","MOV Muxer");
#endif

        if( diaFactoryRun(title,NB_TABS,tabs))
        {
            muxerConfig.muxerType=(MP4_MUXER_TYPE)fmt;
            muxerConfig.optimize=(MP4_MUXER_OPTIMIZE)opt;
            muxerConfig.forceAspectRatio=force;
            muxerConfig.aspectRatio=(MP4_MUXER_DAR)dar;
            muxerConfig.displayWidth=diw;
            muxerConfig.rotation=(MP4_MUXER_ROTATION)rot;
            muxerConfig.clockfreq=(MP4_MUXER_CLOCK_FREQUENCIES)clk;
            return true;
        }
        return false;
}


