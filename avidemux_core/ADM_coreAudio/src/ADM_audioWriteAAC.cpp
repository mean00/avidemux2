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
	96000, 88200, 64000, 48000,
	44100, 32000, 24000, 22050,
	16000, 12000, 11025,  8000,
	0,     0,     0,     0 
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
    int profileMinus1=0;
    if(stream->getExtraData(&l,&d))
    {
        if(l>0)
        {
            profileMinus1=d[0]>>5;
            if(profileMinus1)
                profileMinus1--;
            ADM_info("AAC profile minus 1= %d\n",profileMinus1);
        }else
            ADM_warning("No valid AAC extra data");
    }else
    {
        ADM_warning("Cannot get profile!\n");
    }
    
    
    int channel=hdr->channels;
    
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
