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
#include "muxerMkv.h"
#define ADM_MINIMAL_UI_INTERFACE
#include "DIA_factory.h"
bool mkvConfigure(void)
{
        bool force=mkvMuxerConfig.forceDisplayWidth;
        uint32_t displayWidth=mkvMuxerConfig.displayWidth;
        uint32_t dar=mkvMuxerConfig.displayAspectRatio;
        if(dar)
            force=false;

        diaElemToggle   alternate(&force,QT_TRANSLATE_NOOP("mkvmuxer","Force display width"));
        diaElemUInteger dWidth(&displayWidth,QT_TRANSLATE_NOOP("mkvmuxer","Display width"),16,65535);
        diaMenuEntry    aspect[]={{OTHER,"----"},{STANDARD,"4:3"},{WIDE,"16:9"},{UNI,"18:9"},{CINEMA,"64:27"}};
        diaElemMenu     menuAspect(&dar,QT_TRANSLATE_NOOP("mkvmuxer","Force Aspect Ratio (DAR)"),5,aspect,"");

        alternate.link(1,&dWidth);
        alternate.link(0,&menuAspect);
        menuAspect.link(aspect,1,&alternate);

        diaElem *tabs[]={&alternate,&dWidth,&menuAspect};
        if( diaFactoryRun(QT_TRANSLATE_NOOP("mkvmuxer","MKV Muxer"),3,tabs))
        {
            mkvMuxerConfig.forceDisplayWidth=force;
            mkvMuxerConfig.displayWidth=displayWidth;
            mkvMuxerConfig.displayAspectRatio=dar;
            return true;
        }
        return false;

}


