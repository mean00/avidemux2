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


typedef struct
{
    const char *fmt;
    uint32_t   bufferSizekBytes;
    uint32_t   muxRatekBits;
    uint32_t   videoRatekBits;
}mpegPsStruct;

const mpegPsStruct psDescriptor[4]=
{
    { "vcd",  40,1400,1152},  // Verify, not sure!
    { "svcd",112,2800,2400},
    { "dvd", 224,11000,9800},
    { "free", 1024,70000,30000}
};
#define PS_NB_MUXERS (sizeof(psDescriptor)/sizeof(mpegPsStruct))

/**
    \fn ffPSConfigure
*/
bool ffPSConfigure(void)
{
        bool tolerance=psMuxerConfig.acceptNonCompliant;
        #define TX(x) &(psMuxerConfig.x)


        diaMenuEntry format[]={{MUXER_VCD,"VCD"},{MUXER_SVCD,"SVCD"},{MUXER_DVD,"DVD"},    
                                {MUXER_FREE,"Free"}};

        diaElemMenu  menuFormat(TX(muxingType),"Muxing Format",4,format,"");
        diaElemToggle alternate(&tolerance,"Allow non compliant stream");
        diaElemUInteger muxRate(TX(muxRatekBits),"Total Muxrate (kbits)",500,80000);
        diaElemUInteger videoRate(TX(videoRatekBits),"Video Muxrate (kbits)",500,80000);
        diaElemUInteger vbvBuffer(TX(bufferSizekBytes),"VBV size (kBytes)",10,500);
        diaElemFrame   frameAdvanced("Advanced");

        frameAdvanced.swallow(&muxRate);
        frameAdvanced.swallow(&videoRate);
        frameAdvanced.swallow(&vbvBuffer);

#define LINK(x,y,z)   menuFormat.link((diaMenuEntry *)(format+x),y,&z);
        
        LINK(3,1,muxRate)
        LINK(3,1,videoRate)
        LINK(3,1,vbvBuffer)
        
        diaElem *tabs[]={&menuFormat,&alternate,&frameAdvanced};
        if( diaFactoryRun(("Mpeg PS Muxer"),3,tabs))
        {
            psMuxerConfig.acceptNonCompliant=tolerance;
            // Override with pre-defined value
            if(psMuxerConfig.muxingType<3)
            {
                const mpegPsStruct *p=&(psDescriptor[psMuxerConfig.muxingType]);
                psMuxerConfig.muxRatekBits    =p->muxRatekBits;
                psMuxerConfig.videoRatekBits  =p->videoRatekBits;
                psMuxerConfig.bufferSizekBytes=p->bufferSizekBytes;
            }
            return true;
        }
        return false;
}
// EOF



