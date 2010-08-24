/***************************************************************************
                   \file       audiofilterFilm2pal.cpp
                             
    
    copyright            : (C) 2002/2009 by mean
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

#include <math.h>

#include "ADM_default.h"

#include "ADM_audioFilter.h"
#include "audiofilter_film2pal.h"

AUDMAudioFilterFilm2Pal::AUDMAudioFilterFilm2Pal(AUDMAudioFilter *previous) : 
            AUDMAudioFilterFilmChange(previous,1001,960)
{
    printf("[Film2Pal] Created\n");
}
AUDMAudioFilterPal2Film::AUDMAudioFilterPal2Film(AUDMAudioFilter *previous) : 
            AUDMAudioFilterFilmChange(previous,960,1001)
{
    printf("[Pal2Film] Created\n");
}
#define CONTECT ((SRC_STATE *)context))
//__________

AUDMAudioFilterFilmChange::AUDMAudioFilterFilmChange(AUDMAudioFilter * instream,uint32_t from, uint32_t to):AUDMAudioFilter (instream)
{
    // The parameter are in sample, we deal with fq
    if(true!=resampler.init(from,to,_wavHeader.channels))
    {
        printf("[AudioFilter Resample] Init failed! \n");
        ADM_assert(0);
    }
    printf("[FilmChange] Creating\n");
};

AUDMAudioFilterFilmChange::~AUDMAudioFilterFilmChange()
{
  
  printf("[FilmChange] Destroying\n");
}
//
//___________________________________________
uint32_t AUDMAudioFilterFilmChange::fill( uint32_t max, float * buffer,AUD_Status *status)
{
  uint32_t len,i,rendered;
  uint32_t chan=_wavHeader.channels;
  
  
  shrink();
  fillIncomingBuffer(status);
  
  len=_tail-_head;
  int nbBlock=max/1001;
  max=nbBlock*960; // Prevent overflow when slowing down

  if(len>max) len=max;
  
  len=len/chan; // in sample

        uint32_t maxSample=max/chan; // in sample
        uint32_t nbOut=0;
        uint32_t nbIn=len;
        uint32_t nbInTaken=0;
        float *from,*to;
        from=_incomingBuffer+_head;
        to=buffer;
        if(true!=resampler.process(from,to, 
                    nbIn,
                    maxSample,
                    &nbInTaken, 
                    &nbOut))
        {
            printf("[FilmChange] EOF ??\n");
            *status=AUD_END_OF_STREAM;
            return 0;
        }
      _head=_head+(nbInTaken*chan);
      return nbOut*chan;
};

//EOF

