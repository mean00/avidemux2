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
#include "ADM_ptrQueue.h"
#include "ADM_audioClock.h"
#include <math.h>

#if 1
#define changeState(x) {state=StreamCopy##x;}
#define aprintf(...) {}
#else
#define changeState(x) {printf("Changing state from %d to %d\n",state,StreamCopy##x);state=StreamCopy##x;}
#define aprintf ADM_info
#endif

extern ADM_Composer *video_body;
#define MAX_SKEW 80000
#define MIN_SKEW 10000
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
                        /**
                         *      \class perfectAudioPacket
                         */
                        class perfectAudioPacket
                        {
                        protected:
                                uint64_t dts;
                                uint32_t size;
                                uint32_t samples;
                                uint8_t  *data;
                        public:
                                perfectAudioPacket(uint8_t *data, uint32_t size, uint64_t dts, uint32_t samples)
                                {
                                    this->data=new uint8_t[size];
                                    memcpy(this->data,data,size);
                                    this->size=size;
                                    this->samples=samples;
                                    this->dts=dts;
                                }
                                ~perfectAudioPacket()
                                {
                                    delete [] data;
                                    data=NULL;
                                    size=0;
                                };
                                bool clone(uint8_t *dst, uint32_t *s, uint64_t *d, uint32_t *sam)
                                {
                                    memcpy(dst,data,size);
                                    *s=size;
                                    *d=dts;
                                    *sam=samples;
                                    return true;
                                }
                        };
        protected:
                        typedef enum
                        {
                            StreamCopyIdle,StreamCopyDuping,StreamCopyFlushing
                        }StreamCopyState;
                        // Needed for duplication
                        ADM_ptrQueue <perfectAudioPacket>       audioQueue;
                        audioClock      *clock;
                        uint64_t        nextDts;
                        StreamCopyState state;
                        uint32_t        channels;
                        bool            dupePacket(uint8_t *data, uint32_t size, uint32_t nbSamples, uint64_t dts)
                        {
                            perfectAudioPacket *packet=new perfectAudioPacket(data,size,dts,nbSamples);
                            audioQueue.pushBack(packet);
                            return true;
                        }
                        bool popBackPacket(uint8_t *data, uint32_t *size, uint32_t *nbSample,uint64_t *dts)
                        {
                            perfectAudioPacket *p= audioQueue.pop();
                            ADM_assert(p);
                            p->clone(data,size,dts,nbSample);
                            delete p;
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
  { // shift <0
      int64_t comp=-shift;
      if(comp<startTime)
      {
          startTime-=comp;
          shift=0;
      }else
      {
          shift=comp-startTime;
          startTime=0;
      }
  }
  ADM_info("Creating audio stream copy with compensation : startTime=%s\n",ADM_us2plain(startTime));
  ADM_info("and shift =%s\n",ADM_us2plain(shift));
  if(needPerfectAudio )
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
                    ADM_audioStreamCopy(input,startTime,shift)
{
    ADM_info("Creating Perfect copy stream, startTime=%s,  shift=%d\n",
                ADM_us2plain(startTime),(int)shift);

    state=StreamCopyIdle;
    clock=new audioClock(in->getInfo()->frequency);
    channels=in->getInfo()->channels;
    nextDts=0;
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
uint8_t         ADM_audioStreamCopyPerfect::getPacket(uint8_t *buffer,uint32_t *size, uint32_t sizeMax,
                                                      uint32_t *nbSample,uint64_t *dts)
{
    
again:
     if(state==StreamCopyFlushing)
      {
            
            if(audioQueue.isEmpty())
            {
                changeState(Idle);
                aprintf("Flushed\n");
                goto again;;
            }
            bool r=popBackPacket(buffer,  size,  nbSample, dts);//int8_t *data, uint32_t *size, uint32_t *nbSample,uint64_t *dts
            *dts=clock->getTimeUs();
            clock->advanceBySample(*nbSample);
            aprintf("Flushing time=%d\n",*dts);
            return r;
       }
        // if we can't get an new packet...
    if(false==in->getPacket(buffer,size,sizeMax,nbSample,dts)) 
    {
        switch(state)
        {
        case StreamCopyDuping:
            {
            state=StreamCopyFlushing;
            goto again;
            break;
            }
     
        case StreamCopyIdle:
                // done processing that
                return false;
                break;
        default: ADM_assert(0);break;
        }
    }
            
    // Either we are duping or just passing data around        
    // fix up the DTS..
    if(*dts!=ADM_NO_PTS)
    {
        int64_t fixup=*dts;
        fixup+=shift;
        if(fixup < (int64_t)startTime) goto again ; // too early...
        fixup-=startTime;
        *dts=(uint64_t)fixup;
    }else
    {
        *dts=clock->getTimeUs();
    }
    aprintf("[PerfectAudio] target=%d current=%d\n",(int)clock->getTimeUs(),(int)*dts);
    switch(state)
    {
        case StreamCopyIdle:
        {
            uint64_t targetTime=clock->getTimeUs();
            int skew=(int) fabs((double)*dts-(double)targetTime);
            aprintf("Skew= %d\n",skew);
            if(skew>MIN_SKEW)
            {
                ADM_warning("[PerfectAudio]Warning skew of %d us\n",skew);
                ADM_warning("[PerfectAudio]Warning target=%d current=%d\n",(int)targetTime,*dts);
            }
            if(skew<MAX_SKEW)
            {
                
                *dts=targetTime; // correct some varying around ideal value
                clock->advanceBySample(*nbSample);
                aprintf("copying : Dts=%d\n",(int)*dts);
                return true;
            }
            if(*dts<targetTime)
            {
                // in the past, drop
                ADM_warning("Audio packet in the past, dropping\n");
                goto again;
            }
            // in the future
            
            nextDts=*dts;
            changeState(Duping);
            dupePacket(buffer,*size,*nbSample,*dts);
            *dts=clock->getTimeUs();
            clock->advanceBySample(*nbSample);
            aprintf("In the future, dts=%d, syncDts=%d\n",*dts,nextDts);
            return true;
        }
            break;
        case StreamCopyDuping:
        {
            uint64_t currentClock=clock->getTimeUs();
            aprintf("Duping clockDts=%d, syncDts=%d\n",currentClock,nextDts);
            if( fabs((double)nextDts-(double)currentClock<MAX_SKEW))
            {
                changeState(Flushing);
            }
            dupePacket(buffer,*size,*nbSample,*dts);
            *dts=currentClock;
            clock->advanceBySample(*nbSample);
            return true;
        }
            break;
        case StreamCopyFlushing:
        {
             if(audioQueue.isEmpty())
             {
                changeState(Idle);
                goto again;
             }
             bool r=popBackPacket(buffer,  size,  nbSample, dts);
             clock->advanceBySample(*nbSample);
             return r;
        }
             break;
    default: ADM_assert(0);break;
    }
    return true;

}
// EOF
