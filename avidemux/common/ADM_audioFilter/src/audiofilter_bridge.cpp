/***************************************************************************
              Convert output of decoder to filter api
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
#include <math.h>

#include "ADM_audioFilter.h"
#include "audiofilter_bridge.h"
#include "ADM_vidMisc.h"

#if 1
#define aprintf(...) {}
#else
#define aprintf printf
#endif

static int _held; // we need to restore the original value of _hold on rewind

/**
 * 
 * @param in
 * @return 
 */
static int64_t  _ms2us(int32_t in)
{
   int64_t us=(int64_t ) in;
   us*=1000LL;
   return us;
}

/**
    \fn AUDMAudioFilter_Bridge
*/
AUDMAudioFilter_Bridge::AUDMAudioFilter_Bridge(ADM_edAudioTrack *incoming,
                          uint32_t startInMs,int32_t shiftMs) : AUDMAudioFilter(NULL)
{
  shiftMs=-shiftMs;
  _incoming=incoming;
  memcpy(&_wavHeader,_incoming->getInfo(),sizeof(_wavHeader));
  // For SBR TEST
  //_wavHeader.frequency*=2;
  _wavHeader.frequency=_incoming->getOutputFrequency();
  uint32_t outChannels=_incoming->getOutputChannels();
  if(outChannels)
      _wavHeader.channels=(uint16_t)outChannels;
  // /For SBR Test
  _startTimeUs=_ms2us(startInMs);
  _shiftUs=_ms2us(shiftMs);
  
  _held=_hold=0;

  ADM_info("[Bridge] Starting with time %s , shift %" PRIi32" ms\n",ADM_us2plain(startInMs*1000LL),-shiftMs);
  ADM_info("[Bridge] Ending with time %s, sample %u\n",ADM_us2plain(_startTimeUs),_hold);
  rewind();
  _incoming->updateHeader();
  memcpy(&_wavHeader,_incoming->getInfo(),sizeof(_wavHeader)); // again
  applyShift();
  rewind();
}
/**
    \fn ~AUDMAudioFilter_Bridge
*/

AUDMAudioFilter_Bridge::~AUDMAudioFilter_Bridge()
{
  printf("[Bridge] Destroying bridge\n");
}
/**
    \fn applyShift
*/
bool AUDMAudioFilter_Bridge::applyShift(void)
{
    if(!_shiftUs) return true;
    if(_shiftUs>0)
    { // If shift is positive, it means we have to go in the future, just increase _startTime
        _startTimeUs+=_shiftUs;
        return true;
    }
    // If shift is negative, we have to go either in the past and/or duplicate frames
    _shiftUs=-_shiftUs;
    if(_startTimeUs>_shiftUs)
    {
        _startTimeUs-=_shiftUs;
    }else
    {
        _shiftUs-=_startTimeUs;
        _startTimeUs=0;
        double dNbSample=_shiftUs;
        dNbSample*=_wavHeader.frequency;
        dNbSample/=1000000.;
        dNbSample*=_wavHeader.channels;
        _hold=(int32_t)dNbSample;
        _held=_hold;
    }
    return true;
}
/**
    \fn rewind
*/
uint8_t AUDMAudioFilter_Bridge::rewind(void)
{
  ADM_info("[AudioBridge] Going to time %s\n",ADM_us2plain(_startTimeUs));
  uint8_t r= _incoming->goToTime(_startTimeUs);
  if(!r) ADM_warning("[AudioBridge] Failed!\n");
  _hold=_held;
  return r;
}
/**
    \fn fill
*/
uint32_t   AUDMAudioFilter_Bridge::fill(uint32_t max,float *output,AUD_Status *status)
{
  ADM_assert(_tail>=_head);
  shrink();
  ADM_assert(_tail>=_head);
  fillIncomingBuffer(status);
  // Now fill output
  // We could probably skip the buffering step
  // one extra memcpy gained
  uint32_t available;
  ADM_assert(_tail>=_head);
  available=_tail-_head;
  if(available>max) available=max;
  memcpy(output,_incomingBuffer.at(_head),available*sizeof(float));
  _head+=available;
  if(!available)
  {
    //printf("[bridge] No data in %u max %u out %u\n",_tail-_head,max,available);
  }
  return available;

}
/**
    \fn fillIncomingBuffer
*/
uint8_t AUDMAudioFilter_Bridge::fillIncomingBuffer(AUD_Status *status)
{
  uint32_t asked,got;
  uint64_t dts;
  static bool endMet=false;
  *status=AUD_OK;
  double sampleToUs=1000000./(_wavHeader.frequency*_wavHeader.channels*sizeof(float));
  // Hysteresis
  if((_tail-_head)<(AUD_PROCESS_BUFFER_SIZE>>2)) // Less than 1/4 full
  {
    int ratelimit=0;
    while ((  _tail < (3*AUD_PROCESS_BUFFER_SIZE)/5)) // Fill up to 3/5--3/4
    {
      // don't ask too much front.
      asked = (3*AUD_PROCESS_BUFFER_SIZE)/4-_tail;
      if(false==_incoming->getPCMPacket(_incomingBuffer.at(_tail), asked, &got,&dts))
      {
          got=0;
          dts=ADM_NO_PTS;
      }
      // silence the audio before the specified delay is over
      if(got && _hold>0)
        memset(_incomingBuffer.at(_tail),0,got*_wavHeader.channels*sizeof(float));

      uint64_t endDts=dts+got*sampleToUs;
      aprintf("StartTime : %s\n",ADM_us2plain(_startTimeUs));
      aprintf("packet start : %s\n",ADM_us2plain(dts));
      aprintf("packet end : %s\n",ADM_us2plain(endDts));
      
      if(dts!=ADM_NO_PTS && endDts < _startTimeUs)
      {
          if(ratelimit<8)
          {
              printf("[fillIncomingBuffer] Packet too early, dropping it... :%s",ADM_us2plain(dts));
              printf("\n start is at :%s\n",ADM_us2plain(_startTimeUs));
          }
          ratelimit++;
          continue;
      }
      if(ratelimit>=8)
      {
          ADM_info("%d consecutive 'packet too early' messages suppressed.\n",ratelimit-7);
          ratelimit=0;
      }
      got*=_wavHeader.channels; // sample->float
      if (!got )
      {
        *status=AUD_END_OF_STREAM;
        if(endMet==false) ADM_warning("[Bridge] End of stream\n");
        endMet=true;
        break;
      }
      endMet=false;
      //printf("Bridge : Packet width DTS=%"PRId64"\n",dts);
      _tail+=got;
      if(_hold>0)
      {
        _hold-=got;
        if(_hold<=0)
        {
          printf("[Bridge] Looping\n");
          _hold=-_hold;
          _tail-=_hold;
          rewind();
          _hold=0;
        }
      }
    }
  }
  return 1;
}
/**
 * 	\fn return _incoming->getChannelMapping();
 * \brief since it is a bridge, we translate the filter channel mapping from the audiostream channel mapping
 */
CHANNEL_TYPE *AUDMAudioFilter_Bridge::getChannelMapping(void) 
{
	ADM_assert(_incoming);
	return _incoming->getChannelMapping();
}
/**
 * \fn getLanguage
 * @return 
 */
 const std::string &AUDMAudioFilter_Bridge::getLanguage(void)
{
    return _incoming->getLanguage();
}
//EOF
