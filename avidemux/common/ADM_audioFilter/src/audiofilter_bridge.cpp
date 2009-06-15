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


#include "ADM_default.h"
#include <math.h>

#include "ADM_audioFilter.h"
#include "audiofilter_bridge.h"

#include "ADM_debugID.h"
#define MODULE_NAME MODULE_AUDIO_FILTER
#include "ADM_debug.h"

/**
    \fn AUDMAudioFilter_Bridge
*/
AUDMAudioFilter_Bridge::AUDMAudioFilter_Bridge(ADM_Composer *incoming,
                          uint32_t startInMs,int32_t shiftMs) : AUDMAudioFilter(NULL)
{
  _incoming=incoming;
  memcpy(&_wavHeader,_incoming->getInfo(),sizeof(_wavHeader));
  _startTime=startInMs;
  shiftMs=-shiftMs;
  _shift=shiftMs;
  _hold=0;
  rewind();
  
  printf("[Bridge] Starting with time %u, shift %d\n",startInMs,-shiftMs);
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
  printf("[Bridge] Going to time %d\n",_startTime);
  uint8_t r= _incoming->goToTime(ttime);
  if(!r) printf("[Bridge] Failed!\n");
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
    printf("[bridge] No data in %u max %u out %u\n",_tail-_head,max,available);
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
  *status=AUD_OK;
  // Hysteresis
  if((_tail-_head)<(AUD_PROCESS_BUFFER_SIZE>>2)) // Less than 1/4 full
  {

    while ((  _tail < (3*AUD_PROCESS_BUFFER_SIZE)/5)) // Fill up to 3/5--3/4
    {
      // don't ask too much front.
      asked = (3*AUD_PROCESS_BUFFER_SIZE)/4-_tail;
      if(_hold)
      {
        int32_t sam;
         
        sam=_hold/_wavHeader.channels;
        sam++;
        sam*=_wavHeader.channels;
        if(asked>sam) asked=sam;
        
      }
      asked/=_wavHeader.channels; // float->samples
      _incoming->getPCMPacket(&(_incomingBuffer[_tail]), asked, &got,&dts);
      got*=_wavHeader.channels; // sample->float
      if (!got )
      {
        *status=AUD_END_OF_STREAM;
        printf("[Bridge] End of stream\n");
        break;
      }
      _tail+=got;
      if(_hold>0)
      {
        _hold-=got;
        if(_hold<=0)
        {
          printf("[Bridge] Looping\n");
          rewind();
          //_tail=_head=0;
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
