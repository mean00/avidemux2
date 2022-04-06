/***************************************************************************
            \file audiofilter_eq
            \brief Equalizer
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

#ifndef AUDM_AUDIO_EQ_H
#define AUDM_AUDIO_EQ_H


class EQ3band
{
    protected:
        double vsa;
        
        // Filter #1 (Low band)
        double  lf;       // Frequency
        double  f1p0;     // Poles ...
        double  f1p1;
        double  f1p2;
        double  f1p3;

        // Filter #2 (High band)
        double  hf;       // Frequency
        double  f2p0;     // Poles ...
        double  f2p1;
        double  f2p2;
        double  f2p3;

        // Sample history buffer
        double  sdm1;     // Sample data minus 1
        double  sdm2;     //                   2
        double  sdm3;     //                   3

        // Gain Controls
        double  lg;       // low  gain
        double  mg;       // mid  gain
        double  hg;       // high gain
        
    public:
        EQ3band(float lowFreq, float highFreq, float sampRate, float lowG, float midG, float highG);
        ~EQ3band();
        float update(float samp);
        void reset();
};

typedef struct
{
    bool enable;
    float lowDB;
    float midDB;
    float highDB;
    float cutOffLM;
    float cutOffMH;
} EQparam;

class AUDMAudioFilterEq : public AUDMAudioFilter
{
    protected:
        bool            bypass;
        EQ3band       * eq[MAX_CHANNELS];
        uint32_t        channels;
        CHANNEL_TYPE    channelMapping[MAX_CHANNELS];
    public:
      static void resetConf(EQparam * cfg);
      ~AUDMAudioFilterEq();
      AUDMAudioFilterEq(AUDMAudioFilter *instream, EQparam * cfg);
      uint32_t   fill(uint32_t max,float *output,AUD_Status *status);
      uint8_t    rewind(void);

};
#endif
