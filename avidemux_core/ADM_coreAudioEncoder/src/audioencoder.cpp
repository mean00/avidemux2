/***************************************************************************
    \file audioencoder.cpp

    copyright            : (C) 2002-6 by mean/gruntster/Mihail 
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
#include "ADM_coreAudio.h"
#include "ADM_audioFilter.h"
#include "audioencoder.h"
#include "ADM_audioCodecEnum.h"
#include "BVector.h"
#include "audioencoderInternal.h"

BVector <ADM_audioEncoder *> ListOfAudioEncoder;

/**

*/
ADM_AudioEncoder::ADM_AudioEncoder(AUDMAudioFilter *in, CONFcouple *setup)
{
    _extraData=NULL;
    _extraSize=0;
    ADM_assert(in);
    _state=AudioEncoderRunning;
    _incoming=in;
    memset(&wavheader,0,sizeof(wavheader));
    tmphead=tmptail=0;
    WAVHeader  *info=in->getInfo();
    // Copy channels etc.. from incoming
    wavheader.channels=info->channels;
    wavheader.frequency=info->frequency;
    tmpbuffer.setSize(2*ADM_AUDIO_ENCODER_BUFFER_SIZE);
}
/**

*/
ADM_AudioEncoder::~ADM_AudioEncoder()
{
    if(_extraData)
    {
        delete [] _extraData;
        _extraData=NULL;
    }
}

/**
    \fn refillBuffer
*/
bool  ADM_AudioEncoder::refillBuffer(int minimum)
{
    uint32_t filler=wavheader.frequency*wavheader.channels;
  uint32_t nb;
  AUD_Status status;
  if(AudioEncoderRunning!=_state) return false;
  while(1)
  {
    ADM_assert(tmptail>=tmphead); 
    if((tmptail-tmphead)>=minimum) return true; // already enough data
  
    if(tmphead && tmptail>filler/2)
    {
      memmove(tmpbuffer.at(0),tmpbuffer.at(tmphead),(tmptail-tmphead)*sizeof(float)); 
      tmptail-=tmphead;
      tmphead=0;
    }
    ADM_assert(filler>tmptail);
    nb=_incoming->fill( (filler-tmptail)/2,tmpbuffer.at(tmptail),&status);
    if(!nb)
    {
      if(status!=AUD_END_OF_STREAM) ADM_assert(0);
      
      if((tmptail-tmphead)<minimum)
      {
        memset(tmpbuffer.at(tmptail),0,sizeof(float)*(minimum-(tmptail-tmphead)));
        tmptail=tmphead+minimum;
        _state=AudioEncoderNoInput;  
        return true;
      }
      else continue;
    } else
      tmptail+=nb;
  }
    return true;
}
/**
 * \fn reorderToPlanar
 * 
 * @param sample_in
 * @param sample_out
 * @param samplePerChannel
 * @param mapIn
 * @param mapOut
 * @return 
 */
bool ADM_AudioEncoder::reorderToPlanar(float *sample_in,float *sample_out,int samplePerChannel,CHANNEL_TYPE *mapIn,CHANNEL_TYPE *mapOut)
{
        // build matrix 
    int channel=wavheader.channels;
    for(int i=0;i<channel;i++)
    {
        int chanIn=-1;
        for(int z=0;z<channel;z++)
            if(mapOut[i]==mapIn[z]) chanIn=z;
        ADM_assert(chanIn!=-1);
        float *in=sample_in+chanIn;
        float *out=sample_out+(i*samplePerChannel);
        for(int j=0;j<samplePerChannel;j++)
        {
            *out++=*in;
            in+=channel;
        }
    }
    return true;
}
/**
 * \fn reorderToPlanar
 * 
 * @param sample_in
 * @param sample_out
 * @param samplePerChannel
 * @param mapIn
 * @param mapOut
 * @return 
 */
bool ADM_AudioEncoder::reorderToPlanar2(float *sample_in,float **sample_out,int samplePerChannel,CHANNEL_TYPE *mapIn,CHANNEL_TYPE *mapOut)
{
        // build matrix 
    int channel=wavheader.channels;
    for(int i=0;i<channel;i++)
    {
        int chanIn=-1;
        for(int z=0;z<channel;z++)        
            if(mapOut[i]==mapIn[z]) 
            {
                chanIn=z;
                break;
            }
        ADM_assert(chanIn!=-1);
        float *in=sample_in+chanIn;
        float *out=sample_out[i];
        for(int j=0;j<samplePerChannel;j++)
        {
            *out++=*in;
            in+=channel;
        }
    }
    return true;
}

/**
 * 
 * @param sample_in
 * @param sample_out
 * @param samplePerChannel
 * @param mapIn
 * @param mapOut
 * @return 
 */
bool ADM_AudioEncoder::reorder(float *sample_in,float *sample_out,int samplePerChannel,CHANNEL_TYPE *mapIn,CHANNEL_TYPE *mapOut)
{
        // build matrix 
        int channel=wavheader.channels;
    for(int i=0;i<channel;i++)
    {
        CHANNEL_TYPE chanin=mapIn[i];
        int chanout=-1;
        for(int j=0;j<channel;j++)
        {
            if(mapOut[j]==chanin)
            {
                chanout=j;
                //printf("Channel %s in source at %d, at exit at %d\n",ADM_printChannel(chanin),i,j);
                break;
            }
        }
        if(chanout==-1)
        {
            ADM_warning("Cannot map channel %d : %s\n",i,ADM_printChannel(chanin));
            continue; 
        }
        float *xin=sample_in+i;
        float *xout=sample_out+chanout;
        for(int x=0;x<samplePerChannel;x++)
        {
            *xout=*xin;
            xout+=channel;
            xin+=channel;
        }
        
    }
    return true;
}
/**
 * \fn getLanguage
 * @return 
 */
const std::string &ADM_AudioEncoder::getLanguage(void)
  {
      return _incoming->getLanguage();
  }
//EOF
