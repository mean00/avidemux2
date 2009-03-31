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
#include "../ADM_editor/ADM_edit.hxx"
class AUDMAudioFilter_Bridge : public AUDMAudioFilter
{
  protected:
    ADM_Composer  *_incoming;
    uint32_t _startTime; /*< Starting time in ms */
    int32_t  _shift;  /*< Shift in Ms */
    int32_t  _hold;   /*< Nb Sample to repeat */
    virtual uint8_t fillIncomingBuffer(AUD_Status *status);
  public:
    AUDMAudioFilter_Bridge(ADM_Composer *incoming, uint32_t startInMs,int32_t shiftMS);
    virtual                ~AUDMAudioFilter_Bridge();
    virtual    uint32_t   fill(uint32_t max,float *output,AUD_Status *status);      // Fill buffer: incoming -> us
                                                                                           // Output MAXIMUM max float value
                                                                                           // Not sample! float!
    virtual    uint8_t    rewind(void)  ;                                              // go back to the beginning
    virtual CHANNEL_TYPE *getChannelMapping(void);
};


#endif

