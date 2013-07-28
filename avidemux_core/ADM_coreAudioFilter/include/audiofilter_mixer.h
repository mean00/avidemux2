/***************************************************************************
            \file audiofilter_mixer
            \brief Mixer
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

#ifndef AUDM_AUDIO_MIXER_H
#define AUDM_AUDIO_MIXER_H
#include "audiofilter_dolby.h"
class AUDMAudioFilterMixer : public AUDMAudioFilter
{
    protected:
        CHANNEL_CONF    _output;
        CHANNEL_CONF    _input;
        // output channel mapping
        CHANNEL_TYPE    outputChannelMapping[MAX_CHANNELS];
        // Dolby specific info
    public:
       ADMDolbyContext dolby;

      ~AUDMAudioFilterMixer();
      AUDMAudioFilterMixer(AUDMAudioFilter *instream,CHANNEL_CONF out);
      uint32_t   fill(uint32_t max,float *output,AUD_Status *status);
      // That filter changes its output channel mapping...
      virtual   CHANNEL_TYPE    *getChannelMapping(void );
      uint8_t  rewind(void)
                {
                        dolby.reset();
                        return AUDMAudioFilter::rewind();
                }
};
#endif
