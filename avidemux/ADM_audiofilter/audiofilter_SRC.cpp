/***************************************************************************
        Sox
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

#include "ADM_audioFilter.h"
#include "audiofilter_SRC.h"
/**
    \fn AUDMAudioFilterSrc

*/
 AUDMAudioFilterSrc::AUDMAudioFilterSrc(AUDMAudioFilter *instream,uint32_t  tgt):AUDMAudioFilter (instream)
{

  targetFrequency=tgt;
  _previous->rewind();     // rewind
  printf("[FilterSrc] Creating from %d Hz to %d Hz\n",_wavHeader.frequency,targetFrequency);
  if(_wavHeader.frequency==targetFrequency)
  {
    engaged=0;
    return;
  }
    int org=_previous->getInfo()->frequency;
    if(true!=resampler.init(org,tgt,_wavHeader.channels))
    {
        printf("[AudioFilter Resample] Init failed! \n");
        engaged=0;
        return;
    }
    engaged=1;
    _wavHeader.frequency= targetFrequency;    
     printf("[AudioFilter Resample] Init done. \n");
};

AUDMAudioFilterSrc::~AUDMAudioFilterSrc()
{
  printf("[AudioFilter Resample] Destroying\n");
};

#define BLK_SIZE 512
//_____________________________________________
uint32_t AUDMAudioFilterSrc::fill(uint32_t max,float *output,AUD_Status *status)
{
  if(!engaged)
  {
    return _previous->fill(max, output,status); 
    
  }
    uint32_t snboutput=0;
    // BLK_SIZE sample will generate blockOut sample*channel
    float blockOutf=BLK_SIZE*_wavHeader.channels*_wavHeader.frequency;
    blockOutf/=_previous->getInfo()->frequency;
    uint32_t blockOut=(uint32_t)(blockOutf);

    // Roundup to the next # channels
    blockOut=(blockOut+_wavHeader.channels-1)/_wavHeader.channels;
    blockOut*=_wavHeader.channels;

    while(max>blockOut)
    {
      // Fill incoming buffer
        shrink();
        fillIncomingBuffer(status);
        if(_head==_tail)
        {
          *status=AUD_END_OF_STREAM;
          return snboutput;
        }
        ADM_assert(_tail>=_head);
        uint32_t nb_in=(_tail-_head)/(_wavHeader.channels); // Nb Sample
        if(nb_in>BLK_SIZE) nb_in=BLK_SIZE;
        if(!nb_in)
        {
          printf("[Resampler]Not enough audio\n");
          return snboutput;
        }
        // We have one BLK_SIZE incoming
        uint32_t maxSample=BLK_SIZE*4; // FIXME!
        uint32_t nbOut=0;
        uint32_t nbIn=nb_in;
        uint32_t nbInTaken=0;
        float *from,*to;
        from=_incomingBuffer+_head;
        to=output;
        if(true!=resampler.process(from,to, 
                    nbIn,
                    maxSample,
                    &nbInTaken, 
                    &nbOut))
        {
                *status=AUD_END_OF_STREAM;
                return snboutput;
        }
        
      _head=_head+(nbInTaken*_wavHeader.channels);
      snboutput+=nbOut*_wavHeader.channels;
      output+=nbOut*_wavHeader.channels;
      max-=nbOut*_wavHeader.channels;
    }
    return snboutput;
}
//EOF
