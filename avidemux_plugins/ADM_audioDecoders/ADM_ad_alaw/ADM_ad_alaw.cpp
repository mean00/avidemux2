/**
    \file ADM_ad_alaw.cpp
    \brief Copied from ADM_ad_ulaw.cpp and libavcodec ulaw decoder
*/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "ADM_default.h"
#include "ADM_ad_plugin.h"

/**
    \class ADM_AudiocodecAlaw
    \brief
*/

class ADM_AudiocodecAlaw : public ADM_Audiocodec
{
public:
                    ADM_AudiocodecAlaw(uint32_t fourcc, WAVHeader *info, uint32_t l, uint8_t *d);
    virtual         ~ADM_AudiocodecAlaw();
    virtual uint8_t run(uint8_t *inptr, uint32_t nbIn, float *outptr, uint32_t *nbOut);
    virtual uint8_t isCompressed(void) { return 1; }
private:
    int16_t         lut[256];
    void            buildLut(void);
};
// Supported formats + declare our plugin
//*******************************************************
static ad_supportedFormat Formats[]={
    {WAV_ALAW,AD_MEDIUM_QUAL}
};

DECLARE_AUDIO_DECODER(ADM_AudiocodecAlaw, // Class
    0,0,1, // Major, minor,patch
    Formats, // Supported formats
    "A-law decoder plugin for avidemux (c) Mean\n"); // Desc
//********************************************************

/**
    \fn ADM_AudiocodecAlaw
    \brief ctor
*/
ADM_AudiocodecAlaw::ADM_AudiocodecAlaw(uint32_t fourcc, WAVHeader *info, uint32_t l, uint8_t *d)
: ADM_Audiocodec(fourcc,*info)
{
    UNUSED_ARG(l);
    UNUSED_ARG(d);

    buildLut();
}
/**
    \fn ~ADM_AudiocodecAlaw
    \brief dtor
*/
ADM_AudiocodecAlaw::~ADM_AudiocodecAlaw()
{

}
/**
    \fn buildLut
    \brief Copied from libavcodec/pcm_tablegen.h (c) Reimar Döffinger.
*/
void ADM_AudiocodecAlaw::buildLut(void)
{
    uint8_t b;
    int16_t s,out;

    for(int i=0;i<256;i++)
    {
        b = i ^ 0x55;
        s = (b & 0x70) >> 4;
        out = b & 0xf;
        if(s)
            out = (out + out + 1 + 32) << (s + 2);
        else
            out = (out + out + 1) << 3;
        if(b & 0x80) out = -out;
        lut[i] = out;
    }
}
/**
    \fn run
    \brief Decode nbIn samples from inptr and write nbOut samples to outptr
*/
uint8_t ADM_AudiocodecAlaw::run(uint8_t *inptr, uint32_t nbIn, float *outptr, uint32_t *nbOut)
{
    *nbOut = nbIn;

    for(uint32_t i=0; i < nbIn; i++)
    {
        uint8_t onebyte = *inptr++;
        *outptr++ = (float)lut[onebyte] / 32768;
    }

    return 1;
}
// EOF
