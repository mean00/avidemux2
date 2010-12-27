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

        diaElemUInteger mux(&muxRate,"Mux rate (MBits/s)",3,60);

        diaElem *tabs[]={&mux};
        if( diaFactoryRun(("TS Muxer"),1,tabs))
        {            
            tsMuxerConfig.muxRateInMBits=muxRate;
            return true;
        }
        return false;
}
// EOF


