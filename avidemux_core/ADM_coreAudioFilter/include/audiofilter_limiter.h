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

#ifndef AUDIO_F_LIMITER_H
#define AUDIO_F_LIMITER_H

#include "ADM_audioFilter.h"
#include "audiofilter_limiter_param.h"

extern const DRCparam drcConfDefault;
/**
    \class AUDMAudioFilterLimiter
*/
class AUDMAudioFilterLimiter : public AUDMAudioFilter
{
  protected:
    uint8_t            filled;
    DRCparam           _param;			
    float              mCircle[DRC_WINDOW];
    float              mLevelCircle[DRC_WINDOW];	
    int                mCircleSize;
    int                mCirclePos;	
    float              mRMSSum;
    float              mThreshold;
    float              mGain;
    float              mAttackFactor;
    float              mDecayFactor;	
    float              mLastLevel;
    float              mGainDB;
    float              AvgCircle(float value);
    void               Follow(float x, float *outEnv, int maxBack);
    float              DoCompression(float value, float env);
    void               drc_cleanup(void);
#define ONE_CHUNK 1000
#define DELIM_WINDOW_SIZE ONE_CHUNK/2		 
    float              follow[DELIM_WINDOW_SIZE];
    float              value[DELIM_WINDOW_SIZE];

  public:
                          AUDMAudioFilterLimiter(AUDMAudioFilter *previous, DRCparam *param);
    virtual                ~AUDMAudioFilterLimiter();
    virtual    uint32_t   fill(uint32_t max,float *output,AUD_Status *status);
};
#endif
