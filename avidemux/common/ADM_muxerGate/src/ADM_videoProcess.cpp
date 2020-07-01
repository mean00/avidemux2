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
#include "ADM_coreUtils.h"
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
    ADM_info("[StreamProcess] Stream %" PRIu32"x%" PRIu32", codec : %s\n",width,height,fcc);
    fourCC=fourCC::get((uint8_t *)fcc);
    frameIncrement=encoder->getFrameIncrement();
    if(frameIncrement)
        averageFps1000=ADM_Fps1000FromUs(frameIncrement);
    else
        averageFps1000=25000;
    timeBaseDen=encoder->getTimeBaseDen();
    timeBaseNum=encoder->getTimeBaseNum();
    printf("[StreamProcess] Average FPS1000=%" PRIu32", timebase: %" PRIu32" / %" PRIu32"\n",averageFps1000,timeBaseNum,timeBaseDen);
    isCFR=false;
    videoDelay=encoder->getEncoderDelay();
    ADM_info("[StreamProcess] Initial video encoder delay: %" PRIu64" ms\n",videoDelay/1000);
    firstPacket=true;
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
    if(firstPacket)
    {
        videoDelay=encoder->getEncoderDelay();
        ADM_info("[StreamProcess] Final video encoder delay: %" PRIu64" ms\n",videoDelay/1000);
        firstPacket=false;
    }
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
     
