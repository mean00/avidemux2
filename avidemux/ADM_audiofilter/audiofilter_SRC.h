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

#ifndef AUDM_AUDIO_SRC_H
#define AUDM_AUDIO_SRC_H
#include "ADM_audioResample.h"
class AUDMAudioFilterSrc : public AUDMAudioFilter
{
  protected:
      uint32_t          targetFrequency;
      uint32_t          engaged;
      ADM_resample      resampler; 
  public:

    ~AUDMAudioFilterSrc();
    AUDMAudioFilterSrc(AUDMAudioFilter *instream,uint32_t  tgt);
    uint32_t   fill(uint32_t max,float *output,AUD_Status *status);
};
#endif
