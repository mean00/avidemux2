/**
    \file ADM_videoProcess
    \brief Wrapper around encoders
    (c) Mean 2008/GPLv2

*/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "ADM_default.h"
#include "ADM_videoProcess.h"
#include "fourcc.h"
#include "ADM_bitstream.h"
/**
    \fn ADM_videoStreamProcess
*/
ADM_videoStreamProcess::ADM_videoStreamProcess(ADM_coreVideoEncoder *encoder)
{
    this->encoder=encoder;
    const char *fcc;

    width=encoder->getWidth();
    height=encoder->getHeight();
    fcc=encoder->getFourcc();
    printf("[StreamProcess] Stream %"LU"x%"LU", codec : %s\n",width,height,fcc);
    fourCC=fourCC::get((uint8_t *)fcc);
    float f=encoder->getFrameIncrement();
    if(f) f=1000000000./f;
        else f=25000;
    averageFps1000=(uint32_t)f;
    printf("[StreamProcess] Average FPS1000=%"LU"\n",averageFps1000);
    isCFR=false;
    bitstream=new ADMBitstream(width*height*4);
    data=new uint8_t [width*height*4];
    bitstream->data=data;
}
/**
    \fn ADM_videoStreamProcess
*/
ADM_videoStreamProcess::~ADM_videoStreamProcess()
{
    if(bitstream)
        delete bitstream;
    bitstream=NULL;
    if(encoder) delete encoder;
    encoder=NULL;
    if(data) delete [] data;
    data=NULL;
}
/**
    \fn getExtraData
*/
bool     ADM_videoStreamProcess::getExtraData(uint32_t *extraLen, uint8_t **extraData)
{

  return encoder->getExtraData(extraLen,extraData);
}
 
/**
    \fn getPacket
*/
bool  ADM_videoStreamProcess::getPacket(uint32_t *len, uint8_t *data, uint32_t maxLen,
                    uint64_t *pts,uint64_t *dts,
                    uint32_t *flags)
{
    if(false==encoder->encode(bitstream)) return false;
    ADM_assert(bitstream->len<maxLen);
    memcpy(data,bitstream->data,bitstream->len);
    *len=bitstream->len;
    *pts=bitstream->pts;
    *dts=bitstream->dts;
    *flags=bitstream->flags;
    return true;
}
/**
    \fn getVideoDuration
*/
uint64_t        ADM_videoStreamProcess::getVideoDuration(void)
{
    return encoder->getTotalDuration();

}
// EOF
     
