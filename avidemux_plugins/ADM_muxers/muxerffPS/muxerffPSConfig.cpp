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
#include "muxerffPS.h"
#define ADM_MINIMAL_UI_INTERFACE
#include "DIA_factory.h"
#include "fourcc.h"
bool ffPSConfigure(void)
{
        uint32_t mux=(uint32_t)psMuxerConfig.muxingType;
        uint32_t tolerance=(uint32_t)psMuxerConfig.acceptNonCompliant;
        diaMenuEntry format[]={{MUXER_VCD,"VCD"},{MUXER_SVCD,"SVCD"},{MUXER_DVD,"DVD"}};

        diaElemMenu  menuFormat(&mux,"Muxing Format",3,format,"");
        diaElemToggle alternate(&tolerance,"Allow non compliant stream");

        diaElem *tabs[]={&menuFormat,&alternate};
        if( diaFactoryRun(("Mpeg PS Muxer"),2,tabs))
        {
            psMuxerConfig.muxingType=(psMuxingType)mux;
            psMuxerConfig.acceptNonCompliant=tolerance;
            return true;
        }
        return false;
}
// EOF


