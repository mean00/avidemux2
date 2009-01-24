/***************************************************************************
   
    copyright            : (C) 2006 by mean
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

#ifndef AUDIO_F_FILM2PAL_H
#define AUDIO_F_FILM2PAL_H

#include "ADM_audioFilter.h"
#include "audiofilter_SRC.h"
#define BLK_SIZE 512

class AUDMAudioFilterFilmChange : public AUDMAudioFilter
{
  protected:
    ADM_resample          resampler;
  public:
                          AUDMAudioFilterFilmChange(AUDMAudioFilter *previous,uint32_t from, uint32_t to);
    virtual                ~AUDMAudioFilterFilmChange();
    virtual    uint32_t   fill(uint32_t max,float *output,AUD_Status *status);
};
class AUDMAudioFilterPal2Film : public AUDMAudioFilterFilmChange
{
  protected:
  public:
                            AUDMAudioFilterPal2Film(AUDMAudioFilter *previous);
};
class AUDMAudioFilterFilm2Pal : public AUDMAudioFilterFilmChange
{
  protected:
  public:
                            AUDMAudioFilterFilm2Pal(AUDMAudioFilter *previous);
};

#endif
