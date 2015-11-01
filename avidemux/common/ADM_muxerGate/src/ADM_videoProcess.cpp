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
    ADM_info("[StreamProcess] Stream %"PRIu32"x%"PRIu32", codec : %s\n",width,height,fcc);
    fourCC=fourCC::get((uint8_t *)fcc);
    frameIncrement=encoder->getFrameIncrement();
    float f=encoder->getFrameIncrement();
    if(f) f=1000000000./f;
        else f=25000;
    averageFps1000=(uint32_t)f;
    printf("[StreamProcess] Average FPS1000=%"PRIu32"\n",averageFps1000);
    isCFR=false;
    videoDelay=encoder->getEncoderDelay();
    printf("[StreamProcess] Video Encoder Delay=%"PRIu64"ms\n",videoDelay/1000);
}
/**
    \fn ADM_videoStreamProcess
*/
ADM_videoStreamProcess::~ADM_videoStreamProcess()
{
    if(encoder) delete encoder;
    encoder=NULL;
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
bool  ADM_videoStreamProcess::getPacket(ADMBitstream *out)
{
    if(false==encoder->encode(out)) return false;
    ADM_assert(out->len<out->bufferSize);
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
     
