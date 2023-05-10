/***************************************************************************
            \file audiofilter_fade
            \brief Fade
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

#ifndef AUDM_AUDIO_FADE_H
#define AUDM_AUDIO_FADE_H


typedef struct
{
    float fadeIn;
    float fadeOut;
    bool videoFilterBridge;
} FADEparam;

class AUDMAudioFilterFade : public AUDMAudioFilter
{
    protected:
        bool            bypass;
        uint32_t        _scanned;
        int64_t         _totalSampleCount;
        int64_t         _currentSampleCount;
        int             fadeInSamples, fadeOutSamples;
        uint32_t        channels;
        uint8_t         preprocess(void);
    public:
      static void resetConf(FADEparam * cfg);
      ~AUDMAudioFilterFade();
      AUDMAudioFilterFade(AUDMAudioFilter *instream, FADEparam * cfg);
      uint32_t   fill(uint32_t max,float *output,AUD_Status *status);
      uint8_t    rewind(void);

};
#endif
