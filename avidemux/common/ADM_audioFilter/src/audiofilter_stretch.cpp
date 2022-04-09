/***************************************************************************
                   \file       audiofilter_stretch.cpp
                             
    
    copyright            : (C) 2002/2009 by mean; 2022 szlldm
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
#include "audiofilter_stretch.h"

AUDMAudioFilterFilm2PalV2::AUDMAudioFilterFilm2PalV2(AUDMAudioFilter *previous) : 
            AUDMAudioFilterStretch(previous,1001.0/960.0,1.0)
{
    printf("[Film2Pal v2] Created\n");
}
AUDMAudioFilterPal2FilmV2::AUDMAudioFilterPal2FilmV2(AUDMAudioFilter *previous) : 
            AUDMAudioFilterStretch(previous,960.0/1001.0,1.0)
{
    printf("[Pal2Film v2] Created\n");
}
//__________

AUDMAudioFilterStretch::AUDMAudioFilterStretch(AUDMAudioFilter * instream,double tempo, double pitch):AUDMAudioFilter (instream)
{
    // The parameter are in sample, we deal with fq
    if(true!=stretcher.init(tempo,pitch,_wavHeader.frequency,_wavHeader.channels))
    {
        printf("[AudioFilter Stretcher] Init failed! \n");
        ADM_assert(0);
    }
    channels = _wavHeader.channels;
    reachedEnd = false;
    printf("[Stretcher] Creating\n");
};

AUDMAudioFilterStretch::~AUDMAudioFilterStretch()
{
    printf("[Stretcher] Destroying\n");
}

uint8_t AUDMAudioFilterStretch::rewind(void)
{
    stretcher.reset();
    reachedEnd = false;
    return AUDMAudioFilter::rewind();
}
//
//___________________________________________
uint32_t AUDMAudioFilterStretch::fill( uint32_t max, float * buffer,AUD_Status *status)
{
    uint32_t len;

    while (1)
    { 
        shrink();
        if (!reachedEnd)
            fillIncomingBuffer(status);
        len=_tail-_head;
        
        if (*status==AUD_END_OF_STREAM)
            reachedEnd = true;
        *status=AUD_OK;

        len=len/channels; // in sample
        uint32_t maxSample=max/channels; // in sample
        uint32_t nbOut=0;
        uint32_t nbIn=len;
        uint32_t nbInTaken=0;
        float *from,*to;
        from=_incomingBuffer.at(_head);
        to=buffer;
        bool last = (reachedEnd && (nbIn==0));
        
        if(true!=stretcher.process(from,to, 
                    nbIn,
                    maxSample,
                    last,
                    &nbInTaken, 
                    &nbOut))
        {
            printf("[Stretcher] EOF ??\n");
            *status=AUD_END_OF_STREAM;
            return 0;
        }
        _head=_head+(nbInTaken*channels);

        if (nbOut > 0)
            return nbOut*channels;
    }
    return 0;
};

//EOF

