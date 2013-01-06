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
#include "ADM_vidMisc.h"
#include "ADM_queue.h"
#include "ADM_audioClock.h"
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
                        int64_t         shift;
        public:
                        ADM_audioStreamCopy(ADM_audioStream *input,uint64_t startTime, int64_t shift);  
virtual                 ~ADM_audioStreamCopy();
virtual WAVHeader      *getInfo(void) {return in->getInfo();};
virtual uint8_t         getPacket(uint8_t *buffer,uint32_t *size, uint32_t sizeMax,uint32_t *nbSample,uint64_t *dts);
virtual bool            getExtraData(uint32_t *l, uint8_t **d);
         uint64_t       getDurationInUs(void);
         bool           isCBR();
};
/**
        \class ADM_audioStreamCopyPerfect
 */
class ADM_audioStreamCopyPerfect : public ADM_audioStreamCopy
{
        protected:
                        typedef enum
                        {
                            StreamCopyIdle,StreamCopyDuping,StreamCopyFlushing
                        }StreamCopyState;
                                    
                        // Needed for duplication
                        bool            needPerfectAudio;
                        bool            firstPacket;
                        audioClock      *clock;
                        StreamCopyState state;
                        uint32_t        channels;
                        bool            dupePacket(uint8_t *data, uint32_t size, uint32_t nbSamples, uint64_t dts)
                        {
                            return true;
                        }
        public:

                        ADM_audioStreamCopyPerfect(ADM_audioStream *input,uint64_t startTime, int64_t shift);  
                        ~ADM_audioStreamCopyPerfect();
virtual uint8_t         getPacket(uint8_t *buffer,uint32_t *size, uint32_t sizeMax,uint32_t *nbSample,uint64_t *dts);
};

/**
 * \fn dtor
 */
ADM_audioStreamCopy::~ADM_audioStreamCopy()
{
}
// Pass Through class, just do the timing
/**
 * \fn ctor
 * @param input
 * @param startTime
 * @param shift
 * @param needPerfectAudio
 */
ADM_audioStreamCopy::ADM_audioStreamCopy(ADM_audioStream *input,uint64_t startTime, int64_t shift) : 
                    ADM_audioStream(NULL,NULL)
{
    ADM_info("Creating copy stream, startTime=%s, shift=%d\n",
                ADM_us2plain(startTime),(int)shift);
    in=input;
    this->startTime=startTime;
    in->goToTime(startTime);
    this->shift=shift;
}
/**
 * \fn isCBR
 * @return 
 */
bool ADM_audioStreamCopy::isCBR()
{
    return in->isCBR();
}
/**
 * getExtraData
 * @param l
 * @param d
 * @return 
 */
bool            ADM_audioStreamCopy::getExtraData(uint32_t *l, uint8_t **d)
{
    return in->getExtraData(l,d); 
}
/**
 * \fn getDurationInUs
 * @return 
 */
uint64_t ADM_audioStreamCopy::getDurationInUs(void)
{
       return in->getDurationInUs();
}
/**
 * \fn getPacket
 * @param buffer
 * @param size
 * @param sizeMax
 * @param nbSample
 * @param dts
 * @return 
 */
uint8_t         ADM_audioStreamCopy::getPacket(uint8_t *buffer,uint32_t *size, uint32_t sizeMax,uint32_t *nbSample,uint64_t *dts)
{
    
again:
    if(false==in->getPacket(buffer,size,sizeMax,nbSample,dts)) 
    {
        // done processing that
       return false;
    }
    if(*dts!=ADM_NO_PTS)
    {
        int64_t corrected=*dts;
        corrected+=shift;
        if(corrected<(int64_t)startTime) goto again; // cant have <0 dts
        *dts=corrected-startTime; 
    }
    return true;

}
/**
        \fn audioCreateCopyStream
*/
ADM_audioStream *audioCreateCopyStream(uint64_t startTime,int32_t shift,ADM_audioStream *input,bool needPerfectAudio)
{
  shift*=-1000; // ms -> us
  // fixup startTime and shift
  if(shift>0) 
  {
        startTime+=shift;
        shift=0;
  }
  else
  {
      int64_t comp=-shift;
      if(comp<startTime)
      {
          startTime-=comp;
          shift=0;
      }else
      {
          shift-=startTime;
          startTime=0;
      }
  }
  ADM_info("Creating audio stream copy with compensation : startTime=%s\n",ADM_us2plain(startTime));
  ADM_info("and shift =%s\n",ADM_us2plain(shift));
  if(needPerfectAudio && false)
        return new ADM_audioStreamCopyPerfect(input,startTime,shift);
  else
        return new ADM_audioStreamCopy(input,startTime,shift);
}
/**
 * \fn ctor
 * @param input
 * @param startTime
 * @param shift
 * @param needPerfectAudio
 */
ADM_audioStreamCopyPerfect::ADM_audioStreamCopyPerfect(ADM_audioStream *input,uint64_t startTime, int64_t shift) : 
                    ADM_audioStreamCopy(in,startTime,shift)
{
    ADM_info("Creating Perfect copy stream, startTime=%s, needPerfectAudio=%d, shift=%d\n",
                ADM_us2plain(startTime),needPerfectAudio,(int)shift);

    this->needPerfectAudio=needPerfectAudio;
    firstPacket=true;
    state=StreamCopyIdle;
    clock=new audioClock(in->getInfo()->frequency);
    channels=in->getInfo()->channels;
}
/**
 * \fn dtor
 */
ADM_audioStreamCopyPerfect::~ADM_audioStreamCopyPerfect()
{
    if(clock)
    {
        delete clock;
        clock=NULL;
    }
    // empty queue 
}
/**
 * \fn getPacket
 * @param buffer
 * @param size
 * @param sizeMax
 * @param nbSample
 * @param dts
 * @return 
 */
uint8_t         ADM_audioStreamCopyPerfect::getPacket(uint8_t *buffer,uint32_t *size, uint32_t sizeMax,uint32_t *nbSample,uint64_t *dts)
{
    
again:
    bool wasFirst=firstPacket;
    firstPacket=false;
    if(state==StreamCopyFlushing)
    {
        // pop packet & go back to  idle if needed
        
    }
    if(false==in->getPacket(buffer,size,sizeMax,nbSample,dts)) 
    {
        // empty queue
        if(state==StreamCopyDuping)
        {
            state=StreamCopyFlushing;
            goto again;
        }
        // done processing that
       return false;
    }
        if(wasFirst && *dts==ADM_NO_PTS)
        {
            ADM_warning("First audio packet has no DTS\n");
        }
        if(state==StreamCopyIdle && wasFirst && *dts!=ADM_NO_PTS)
        {
            // should we engage duping mode ?
            if(0)
            {
                state=StreamCopyDuping;
                // Dupe packet
                dupePacket(buffer,*size,*nbSample,*dts);
                *dts=0;
                clock->setTimeUs(0);
                clock->advanceBySample(*nbSample/channels);
                return true;
            }
        if(*dts!=ADM_NO_PTS)
        {

        }
        int64_t corrected=*dts;
        corrected+=shift;
        if(corrected<(int64_t)startTime) goto again; // cant have <0 dts
        *dts=corrected-startTime; 
    }
    return true;

}
// EOF
