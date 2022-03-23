/***************************************************************************
            \file audiofilter_channels
            \brief Channel manipulations
              (c) 2022 szlldm
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef AUDM_AUDIO_CHANNELS_H
#define AUDM_AUDIO_CHANNELS_H

typedef struct
{
    float chGainDB[ADM_CH_LAST];
    int   chDelayMS[ADM_CH_LAST];
    bool  enableRemap;
    int remap[9];   // fL, fR, fC, sL, sR, rL, rR, rC, LFE

} CHANSparam;

extern const CHANSparam channelsConfDefault;

class AUDMAudioFilterChannels : public AUDMAudioFilter
{
    protected:
        bool            bypass;
        float           chGain[ADM_CH_LAST];
        int             chDelay[ADM_CH_LAST];
        float         * delayLine[ADM_CH_LAST];
        int             delayPtr[ADM_CH_LAST];
        uint32_t        channels;
        CHANNEL_TYPE    channelMapping[MAX_CHANNELS];
        int             channelReMapping[MAX_CHANNELS];

        CHANNEL_TYPE remapToADMChannel(int r);
    public:
      static void resetConf(CHANSparam * cfg);
      ~AUDMAudioFilterChannels();
      AUDMAudioFilterChannels(AUDMAudioFilter *instream, CHANSparam * cfg);
      uint32_t   fill(uint32_t max,float *output,AUD_Status *status);
      uint8_t    rewind(void);

};
#endif
