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

/**
    \fn AUDMAudioFilter_Bridge
*/
AUDMAudioFilter_Bridge::AUDMAudioFilter_Bridge(ADM_edAudioTrack *incoming,
                          uint32_t startInMs,int32_t shiftMs) : AUDMAudioFilter(NULL)
{
  _incoming=incoming;
  memcpy(&_wavHeader,_incoming->getInfo(),sizeof(_wavHeader));
  // For SBR TEST
  //_wavHeader.frequency*=2;
  _wavHeader.frequency=_incoming->getOutputFrequency();
  // /For SBR Test
  _startTime=startInMs;
  shiftMs=-shiftMs;
  _shift=shiftMs;
  _hold=0;
  rewind();
  
  ADM_info("[Bridge] Starting with time %"PRIu32" ms, shift %"PRIi32" ms\n",startInMs,-shiftMs);
  // If shiftMS is > 0, it means we have to go in the future, just increse _startTime
  if(shiftMs>0)
  {
    _startTime+=_shift;
  }
  else if(shiftMs<0) // In that case we have to go either in the past and/or duplicate frames
  {
    shiftMs=-shiftMs;
    if(_startTime>shiftMs)  
    {
      _startTime-=shiftMs;
    }else
    {
      double nbSample;
      
      shiftMs-=_startTime;
      _startTime=0;
      nbSample=shiftMs;
      nbSample*=_wavHeader.frequency; 
      nbSample/=1000.;
      nbSample*=_wavHeader.channels;
      _hold=(int32_t)nbSample;
    }
    
  }
  printf("[Bridge] Ending with time %u, sample %u\n",_startTime,_hold);
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
    \fn rewind
*/
uint8_t AUDMAudioFilter_Bridge::rewind(void)
{
uint64_t ttime=_startTime;
  ttime*=1000; // ms->us
  ADM_info("[AudioBridge] Going to time %d\n",_startTime);
  uint8_t r= _incoming->goToTime(ttime);
  if(!r) ADM_warning("[AudioBridge] Failed!\n");
  return r;
}
/**
    \fn fill
*/
uint32_t   AUDMAudioFilter_Bridge::fill(uint32_t max,float *output,AUD_Status *status)
{
  uint32_t asked,asked2,total=0;
  //
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
  memcpy(output,&(_incomingBuffer[_head]),available*sizeof(float));
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

    while ((  _tail < (3*AUD_PROCESS_BUFFER_SIZE)/5)) // Fill up to 3/5--3/4
    {
      // don't ask too much front.
      asked = (3*AUD_PROCESS_BUFFER_SIZE)/4-_tail;
      asked/=_wavHeader.channels; // float->samples
      _incoming->getPCMPacket(&(_incomingBuffer[_tail]), asked, &got,&dts);
      uint64_t endDts=dts+got*sampleToUs;
      aprintf("StartTime : %s\n",ADM_us2plain(_startTime*1000));
      aprintf("packet start : %s\n",ADM_us2plain(dts));
      aprintf("packet end : %s\n",ADM_us2plain(endDts));
      
      if(dts!=ADM_NO_PTS && endDts < _startTime*1000)
      {
          ADM_info("Packet too early, dropping it...");
          continue;
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
//EOF
