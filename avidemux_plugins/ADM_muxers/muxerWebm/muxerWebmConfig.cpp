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
#include "muxerWebm.h"
#define ADM_MINIMAL_UI_INTERFACE
#include "DIA_factory.h"
#include "fourcc.h" 
bool WebmConfigure(void)
{
        bool force=WebmMuxerConfig.forceDisplayWidth;
        uint32_t displayWidth=(uint32_t)WebmMuxerConfig.displayWidth;

        diaElemToggle   alternate(&force,QT_TRANSLATE_NOOP("webmmuxer","Force display width"));
        diaElemUInteger dWidth(&displayWidth,QT_TRANSLATE_NOOP("webmmuxer","Display width"),16,65535);

        diaElem *tabs[]={&alternate,&dWidth};
        if( diaFactoryRun(QT_TRANSLATE_NOOP("webmmuxer","Webm Muxer"),2,tabs))
        {
            WebmMuxerConfig.forceDisplayWidth=(bool)force;
            WebmMuxerConfig.displayWidth=displayWidth;
            return true;
        }
        return false;

}


