/***************************************************************************
         Bridge
         
         This file is a bridge from output of decoder to filter api
         
 ***************************************************************************/
 
 
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef AUDM_BRIDGE_H
#include "audioencoder.h"
#include "ADM_audioCodecEnum.h"
#include "ADM_audioStream.h"
#include "ADM_edit.hxx"
#include "ADM_edAudioTrack.h"
class AUDMAudioFilter_Bridge : public AUDMAudioFilter
{
  protected:
    ADM_edAudioTrack    *_incoming;
    uint64_t            _startTimeUs; /*< Starting time in us */
    int64_t             _shiftUs;  /*< Shift in Ms */
    int32_t             _hold;   /*< Nb Sample to repeat */
    virtual uint8_t             fillIncomingBuffer(AUD_Status *status);
  public:
                                AUDMAudioFilter_Bridge(ADM_edAudioTrack *incoming, 
                                                uint32_t startInMs,int32_t shiftMS);
    virtual                     ~AUDMAudioFilter_Bridge();
    virtual    uint32_t         fill(uint32_t max,float *output,AUD_Status *status);      // Fill buffer: incoming -> us
                                                                                           // Output MAXIMUM max float value
                                                                                           // Not sample! float!
    virtual    uint8_t          rewind(void)  ;                                              // go back to the beginning
    virtual CHANNEL_TYPE        *getChannelMapping(void);
    virtual const std::string   &getLanguage(void);
};


#endif

