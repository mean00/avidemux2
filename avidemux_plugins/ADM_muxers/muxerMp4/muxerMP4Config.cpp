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
#include "fourcc.h"
bool mp4Configure(void)
{
        uint32_t fmt=(uint32_t)muxerConfig.muxerType;
        uint32_t alt=(uint32_t)muxerConfig.useAlternateMp3Tag;
        diaMenuEntry format[]={{MP4_MUXER_MP4,"MP4"},{MP4_MUXER_PSP,"PSP"}};
        diaElemMenu  menuFormat(&fmt,"Muxing Format",2,format,"");
        diaElemToggle alternate(&alt,"Use alternate MP3 tag");

        diaElem *tabs[]={&menuFormat,&alternate};
        if( diaFactoryRun(("MP4 Muxer"),2,tabs))
        {
            muxerConfig.muxerType=(MP4_MUXER_TYPE)fmt;
            muxerConfig.useAlternateMp3Tag=alt;
            return true;
        }
        return false;
}


