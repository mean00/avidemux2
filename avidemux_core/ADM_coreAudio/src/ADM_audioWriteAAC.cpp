/**
    \file  ADM_audioWriteAAC
    \brief Write AAC packets inside ADTS container
    copyright            : (C) 2016 by mean
    email                : fixounet@free.fr
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
#include "ADM_audioStream.h"
#include "ADM_audioWriteAAC.h"

/**
 */
static 	int aacSampleRate[16]=
{
    96000,  88200,  64000,  48000,
    44100,  32000,  24000,  22050,
    16000,  12000,  11025,  8000,
    7350,   0,      0,      0
};

static const char *audioObjectTypeNames[45]=
{
    "AAC Main",         "AAC LC",           "AAC SSR",          "AAC LTP",
    "AAC SBR",          "AAC Scalable",     "TwinVQ",           "CELP",
    "HXVC",             "Reserved",         "Reserved",         "TTSI",
    "Main Synthesis",   "Wavetable Synthesis", "General MIDI",  "Algorithmic Synthesis and Audio Effects",
    "ER AAC LC",        "Reserved",         "ER AAC LTP",       "ER AAC Scalable",
    "ER TwinVQ",        "ER BSAC",          "ER AAC LD",        "ER CELP",
    "ER HVXC",          "ER HILN",          "ER Parametric",    "SSC",
    "AAC PS",           "MPEG Surround",    "(Escape value)",   "Layer-1",
    "Layer-2",          "Layer-3",          "DST",              "ALS",
    "SLS",              "SLS non-core",     "ER AAC ELD",       "SMR Simple",
    "SMR Main",         "USAC (no SBR)",    "SAOC",             "LD MPEG Surround",
    "USAC"
};

/**
 * \fn frequency2index
 * @param frequency
 * @return 
 */
int frequency2index(int frequency)
{
    
    int dex=0;
    while(aacSampleRate[dex])
    {
        if(aacSampleRate[dex]==frequency)
            return dex;
        dex++;
    }
    return -1;
    
}

/**
    \fn ctor
*/
ADM_audioWriteAAC::ADM_audioWriteAAC()
{

}
/**
    \fn writeHeader
*/
bool ADM_audioWriteAAC::writeHeader(ADM_audioStream *stream)
{

          return true;
}


/**
    \fn close
*/

bool ADM_audioWriteAAC::close(void)
{
    return ADM_audioWrite::close();
}
/**
    \fn init
    \brief write wavHeader
*/

bool ADM_audioWriteAAC::init(ADM_audioStream *stream, const char *fileName)
{
    WAVHeader *hdr=stream->getInfo();
    if(hdr->encoding!=WAV_AAC)
    {
        ADM_warning("Not AAC!\n");
        return false;
    }
    int samplingFrequencyIndex=frequency2index(hdr->frequency);
    if(samplingFrequencyIndex==-1)
    {
        ADM_warning("Invalid frequency \n");
        return false;
    }
    uint32_t l;
    uint8_t *d;
    int profileMinus1 = 1; // AAC Low Complexity - 1
    if(stream->getExtraData(&l,&d))
    {
        int fqIdxFromConfig = samplingFrequencyIndex;
        if(l < 2)
        {
            ADM_warning("Invalid AAC extradata.\n");
            goto noextra;
        }
        profileMinus1 = d[0]>>3;
        if(profileMinus1 == 31)
        {
            if(l < 3)
            {
                ADM_warning("Invalid AAC extradata.\n");
                goto noextra;
            }
            profileMinus1 += (d[1]>>2) & 0x3F;
            fqIdxFromConfig = ((d[1]&3)<<2)+(d[2]>>6);
        }else
        {
            fqIdxFromConfig = ((d[0]&7)<<1)+(d[1]>>7);
        }
        if(profileMinus1 > 45)
        {
            ADM_warning("Invalid AAC audio object type code.\n");
            goto noextra;
        }
        if(profileMinus1)
            profileMinus1--;
        if(profileMinus1 > 3)
        {
            ADM_warning("Audio object type %s not representable in ADTS, faking AAC LC.\n", audioObjectTypeNames[profileMinus1]);
            profileMinus1 = 1;
        }
        if(fqIdxFromConfig == 15)
        {
            ADM_warning("Explicitly specified sampling frequency is not handled.\n");
        }else if(fqIdxFromConfig < 13 && samplingFrequencyIndex!=fqIdxFromConfig)
        {
            ADM_warning("Using frequency index from extradata %d = %d Hz (audio track says %d = %d Hz)\n",
                fqIdxFromConfig, aacSampleRate[fqIdxFromConfig],
                samplingFrequencyIndex, aacSampleRate[samplingFrequencyIndex]);
            samplingFrequencyIndex = fqIdxFromConfig;
        }
    }else
    {
        ADM_warning("Cannot get AAC extradata to extract profile!\n");
    }
noextra:
    int channel=hdr->channels;

    ADM_info("Using AAC audio object type minus one = %d: %s\n", profileMinus1, audioObjectTypeNames[profileMinus1]);

    aacHeader[0]=0xff;
    aacHeader[1]=0xf1;
    aacHeader[2]=(profileMinus1<<6);
    aacHeader[2]|=(samplingFrequencyIndex)<<2;
    aacHeader[2]|=channel>>2;
    aacHeader[3]=(channel&3)<<6; // Channel configuration+frameLength
    aacHeader[4]=0; // frame length
    aacHeader[5]=0; // frame length + buffer fullness
    aacHeader[6]=0; // buffer fullness  + only one frame
    
    if(false==ADM_audioWrite::init(stream,fileName)) return false;
    return true;
}


/**
 * \fn write
 * \brief incoming is one packet
*/
bool ADM_audioWriteAAC::write(uint32_t size, uint8_t *buffer)
{
    int totalLength=size+7;
      // update header
      aacHeader[3]&=0xc0;
      aacHeader[3]|=totalLength>>11;
      aacHeader[4]=totalLength>>3;
      aacHeader[5]=(totalLength&7)<<5;
      // Buffer fullness
      fwrite(aacHeader,7,1,_file);
      return ADM_audioWrite::write(size,buffer);
}

//EOF
