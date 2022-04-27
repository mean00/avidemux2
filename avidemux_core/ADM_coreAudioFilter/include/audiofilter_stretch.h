/***************************************************************************
   
    copyright            : (C) 2006 by mean, 2022 szlldm
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

#ifndef AUDIO_F_STRETCH_H
#define AUDIO_F_STRETCH_H

#include "ADM_audioFilter.h"
#include "ADM_audioStretch.h"
#define BLK_SIZE 512

class AUDMAudioFilterStretch : public AUDMAudioFilter
{
  protected:
    ADM_audioStretch      stretcher;
    uint32_t              channels;
    bool                  reachedEnd;
  public:
                          AUDMAudioFilterStretch(AUDMAudioFilter *previous,double tempo, double pitch);
    virtual                ~AUDMAudioFilterStretch();
               uint8_t    rewind(void);
    virtual    uint32_t   fill(uint32_t max,float *output,AUD_Status *status);
};

class AUDMAudioFilterPal2FilmV2 : public AUDMAudioFilterStretch
{
  protected:
  public:
                            AUDMAudioFilterPal2FilmV2(AUDMAudioFilter *previous);
};
class AUDMAudioFilterFilm2PalV2 : public AUDMAudioFilterStretch
{
  protected:
  public:
                            AUDMAudioFilterFilm2PalV2(AUDMAudioFilter *previous);
};

#endif
