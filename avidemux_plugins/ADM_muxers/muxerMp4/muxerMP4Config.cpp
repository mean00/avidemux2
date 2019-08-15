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

bool mp4Configure(void)
{
        uint32_t fmt=(uint32_t)muxerConfig.muxerType;
        uint32_t dar=(uint32_t)muxerConfig.aspectRatio;
        uint32_t rot=(uint32_t)muxerConfig.rotation;
        uint32_t opt=(uint32_t)muxerConfig.optimize;
        bool force=muxerConfig.forceAspectRatio;
        diaMenuEntry format[]={{MP4_MUXER_MP4,"MP4"},{MP4_MUXER_PSP,"PSP"}};
        diaElemMenu  menuFormat(&fmt,QT_TRANSLATE_NOOP("mp4muxer","Muxing Format"),2,format,"");
        diaMenuEntry streamOpt[]={
            {MP4_MUXER_OPT_NONE,QT_TRANSLATE_NOOP("mp4muxer","No optimization")},
            {MP4_MUXER_OPT_FASTSTART,QT_TRANSLATE_NOOP("mp4muxer","Move index to the beginning of the file")},
            {MP4_MUXER_OPT_FRAGMENT,QT_TRANSLATE_NOOP("mp4muxer","Use fragmentation")}
        };
        diaElemMenu  menuOptimize(&opt,QT_TRANSLATE_NOOP("mp4muxer","Optimize for Streaming"),3,streamOpt,"");
        diaElemToggle forceAR(&force,QT_TRANSLATE_NOOP("mp4muxer","Force aspect ratio"));
        diaMenuEntry aspect[]={{STANDARD,"4:3"},{WIDE,"16:9"},{UNI,"18:9"},{CINEMA,"64:27"}};
        diaElemMenu  menuAspect(&dar,QT_TRANSLATE_NOOP("mp4muxer","Aspect Ratio (DAR)"),4,aspect,"");
        diaMenuEntry rotation[]={
            {MP4_MUXER_ROTATE_0,QT_TRANSLATE_NOOP("mp4muxer","Do not rotate")},
            {MP4_MUXER_ROTATE_90,QT_TRANSLATE_NOOP("mp4muxer","90°")},
            {MP4_MUXER_ROTATE_180,QT_TRANSLATE_NOOP("mp4muxer","180°")},
            {MP4_MUXER_ROTATE_270,QT_TRANSLATE_NOOP("mp4muxer","270°")}
        };
        diaElemMenu menuRotation(&rot,QT_TRANSLATE_NOOP("mp4muxer","Rotate video"),4,rotation,"");
        diaElem *tabs[]={&menuFormat,&menuOptimize,&forceAR,&menuAspect,&menuRotation};

        if( diaFactoryRun(QT_TRANSLATE_NOOP("mp4muxer","MP4 Muxer"),5,tabs))
        {
            muxerConfig.muxerType=(MP4_MUXER_TYPE)fmt;
            muxerConfig.optimize=(MP4_MUXER_OPTIMIZE)opt;
            muxerConfig.forceAspectRatio=force;
            muxerConfig.aspectRatio=(MP4_MUXER_DAR)dar;
            muxerConfig.rotation=(MP4_MUXER_ROTATION)rot;
            return true;
        }
        return false;
}


