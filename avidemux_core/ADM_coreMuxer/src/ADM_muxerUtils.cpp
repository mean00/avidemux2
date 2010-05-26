/***************************************************************************
                          ADM_muxerUtils.cpp  -  description
                             -------------------
    copyright            : (C) 2008 by mean
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
#define __STDC_CONSTANT_MACROS  1 // Lavcodec crap
#define __STDC_LIMIT_MACROS 1
#include "ADM_default.h"
#include "ADM_muxerInternal.h"
#include "ADM_muxerUtils.h"
#include "fourcc.h"
extern const char *getStrFromAudioCodec( uint32_t codec);
/**
    \fn rescaleFps
    \brief Rescale fps to be accurate (i.e. 23.976 become 24000/1001)

*/
void  rescaleFps(uint32_t fps1000, AVRational *rational)
{
    switch(fps1000)
    {
    case 23976 :
    {
        rational->num=1001;
        rational->den=24000;
        break;
    }
    case 29970 :
    {
        rational->num=1001;
        rational->den=30000;
        break;
    }
    default:
    rational->num=1000;
    rational->den=fps1000;
    }
    printf("[MP3] TimeBase for video %d/%d\n",rational->num,rational->den);
}
/**
        \fn rescaleLavPts
        \brief Rescale PTS/DTS the lavformat way, i.e. relative to the scale.
*/
uint64_t rescaleLavPts(uint64_t us, AVRational *scale)
{
#ifndef INT64_C
#define INT64_C(x) (uint64_t)(x##LL)
#endif
     if(us==ADM_NO_PTS) return AV_NOPTS_VALUE;  // AV_NOPTS_VALUE
    double db=(double)us;
    double s=scale->den;

     db*=s;
     db=(db)/1000000.; // in seconds
    uint64_t i=(uint64_t)db; // round up to the closer num value
    i=(i+scale->num-1)/scale->num;
    i*=scale->num;
    return i;
}
/**
    \fn     initUI
    \brief  initialize the progress bar
*/
bool     ADM_muxer::initUI(const char *title)
{
        float f=(float)vStream->getAvgFps1000();
        f=1000./f;
        f*=1000000;
        videoIncrement=(uint64_t)f;  // Video increment in AVI-Tick
        videoDuration=vStream->getVideoDuration();

        encoding=createEncoding(videoDuration);
        // Set video stream etc...
        encoding->setVideoCodec(fourCC::tostring(vStream->getFCC()));
        if(!nbAStreams) encoding->setAudioCodec("None");
                else    encoding->setAudioCodec(getStrFromAudioCodec(aStreams[0]->getInfo()->encoding));
        return true;
}
/**
        \fn updateUI
        \brief Update the progress bar
        @return false if abort request, true if keep going
*/

bool     ADM_muxer::updateUI(void)
{
            ADM_assert(encoding);
            encoding->refresh();
            if(!encoding->isAlive()) 
            {
                return false;
            }
            return true;
}
/**
        \fn closeUI
*/

bool     ADM_muxer::closeUI(void)
{
        if(encoding) delete encoding;
        encoding=NULL;
        return true;
}
// EOF
