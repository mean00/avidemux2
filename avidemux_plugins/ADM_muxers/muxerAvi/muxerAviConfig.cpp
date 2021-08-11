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
#include "muxerAvi.h"
#define ADM_MINIMAL_UI_INTERFACE
#include "DIA_factory.h"
#include "fourcc.h"
extern "C" bool AviConfigure(void)
{
        uint32_t fmt=(uint32_t)muxerConfig.odmlType;
        diaMenuEntry format[]={
            {AVI_MUXER_TYPE1, QT_TRANSLATE_NOOP("avimuxer","AVI"), NULL},
            {AVI_MUXER_AUTO, QT_TRANSLATE_NOOP("avimuxer","Auto"),
                QT_TRANSLATE_NOOP("avimuxer","Start as legacy AVI and switch to OpenDML mode when approaching the 4 GiB file size limit")},
            {AVI_MUXER_TYPE2, QT_TRANSLATE_NOOP("avimuxer","OpenDML"), NULL}
        };
        diaElemMenu  menuFormat(&fmt,QT_TRANSLATE_NOOP("avimuxer","Muxing Format"),3,format,"");

        diaElem *tabs[]={&menuFormat};
        if( diaFactoryRun(QT_TRANSLATE_NOOP("avimuxer","Avi Muxer"),1,tabs))
        {
            muxerConfig.odmlType=fmt;
            return true;
        }
        return false;
}


