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
#include "muxerffTS.h"
#define ADM_MINIMAL_UI_INTERFACE
#include "DIA_factory.h"
#include "fourcc.h"
extern ts_muxer tsMuxerConfig;
bool ffTSConfigure(void)
{
        uint32_t muxRate=(uint32_t)tsMuxerConfig.muxRateInMBits;
        bool     vbr=tsMuxerConfig.vbr;

        diaElemToggle   v(&vbr,"VBR muxing");
        diaElemUInteger mux(&muxRate,"Mux rate (MBits/s)",3,60);

        v.link(0,&mux);

        diaElem *tabs[]={&v,&mux};
        if( diaFactoryRun(("TS Muxer"),2,tabs))
        {            
            tsMuxerConfig.muxRateInMBits=muxRate;
            tsMuxerConfig.vbr=vbr;
            return true;
        }
        return false;
}
// EOF


