/***************************************************************************
            \file audiofilter_encoder.cpp
            \brief Generate a access class = to the output of encoder + filterchain
              (c) 2006 Mean , fixounet@free.fr
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "ADM_cpp.h"
using std::string;
#include "ADM_default.h"
#include "ADM_edit.hxx"
#include <math.h>

extern ADM_Composer *video_body;

/**
        \fn ADM_audioStream
        \brief Base class for audio stream

*/
class ADM_audioStreamCopy : public ADM_audioStream
{
        protected:
                        ADM_audioStream *in;
                        uint64_t        startTime;
        public:

                       ADM_audioStreamCopy(ADM_audioStream *input,uint64_t startTime, int64_t shift);  
virtual                 WAVHeader                *getInfo(void) {return in->getInfo();};
virtual uint8_t         getPacket(uint8_t *buffer,uint32_t *size, uint32_t sizeMax,uint32_t *nbSample,uint64_t *dts);
//virtual bool            goToTime(uint64_t nbUs);
virtual bool             getExtraData(uint32_t *l, uint8_t **d);
         uint64_t        getDurationInUs(void);
         bool            isCBR();
};
// Pass Through class, just do the timing

ADM_audioStreamCopy::ADM_audioStreamCopy(ADM_audioStream *input,uint64_t startTime, int64_t shift) : 
                    ADM_audioStream(NULL,NULL)
{
    in=input;
    this->startTime=startTime;
    in->goToTime(startTime);
#warning handle shift ???
}

bool ADM_audioStreamCopy::isCBR()
{
    return in->isCBR();
}

bool            ADM_audioStreamCopy::getExtraData(uint32_t *l, uint8_t **d)
{
    return in->getExtraData(l,d); 
}
uint64_t ADM_audioStreamCopy::getDurationInUs(void)
{
       return in->getDurationInUs();
}
uint8_t         ADM_audioStreamCopy::getPacket(uint8_t *buffer,uint32_t *size, uint32_t sizeMax,uint32_t *nbSample,uint64_t *dts)
{
again:
    if(false==in->getPacket(buffer,size,sizeMax,nbSample,dts)) return false;
    if(*dts!=ADM_NO_PTS && *dts<startTime) goto again;
    *dts=*dts-startTime;
    return true;

}
/**
        \fn audioCreateCopyStream
*/
ADM_audioStream *audioCreateCopyStream(uint64_t startTime,int32_t shift,ADM_audioStream *input)
{
  return new ADM_audioStreamCopy(input,startTime,shift);
}

// EOF
