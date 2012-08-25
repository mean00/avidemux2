/***************************************************************************
                          audioeng_6dbpress.cpp  -  description
                             -------------------
	Derived from audactity compressor
	see http://audacity.sf.net
 ***************************************************************************/
/**********************************************************************

  Audacity: A Digital Audio Editor

  Compressor.cpp

  Dominic Mazzoni

  Steve Jolly made it inherit from EffectSimpleMono.
  GUI added and implementation improved by Dominic Mazzoni, 5/11/2003.

**********************************************************************/
 
 
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
#include "audiofilter_limiter.h"

const DRCparam drcConfDefault=
{
  1,
  0.001,//double   mFloor;
  0.2, //double   mAttackTime;
  1.0, //double   mDecayTime;
  2.0, //double   mRatio;
  -12.0 ,//double   mThresholdDB;
};


AUDMAudioFilterLimiter::AUDMAudioFilterLimiter(AUDMAudioFilter *previous, DRCparam *param):AUDMAudioFilter (previous)
{
uint32_t nbChan=previous->getInfo()->channels;
#define AMP 4
    _previous->rewind();
    memcpy(&_param,param,sizeof(_param));
//
// The base is 100 ms
//

    mCircleSize=DRC_WINDOW;
    mCircleSize=mCircleSize-(mCircleSize%nbChan);
    drc_cleanup();
    ADM_info("[DRC] Created DRC:%u Window:%u nbChan %u\n",mCircleSize,DRC_WINDOW,nbChan);

};
void AUDMAudioFilterLimiter::drc_cleanup(void)
{
  float mCurRate=(float)_wavHeader.frequency;
   
  for(int j=0; j<mCircleSize; j++) {
      mCircle[j] = 0.0;
      mLevelCircle[j] = _param.mFloor;
   }
   mCirclePos = 0;
   mRMSSum = 0.0;

   mGainDB = ((_param.mThresholdDB*-0.7) * (1 - 1/_param.mRatio));
   if (mGainDB < 0)
      mGainDB = 0;

   mThreshold = pow(10.0, _param.mThresholdDB/10); // factor of 10 because it's power

   if (_param.mUseGain)
      mGain = powf(10.0, mGainDB/20); // factor of 20 because it's amplitude
   else
      mGain = 1.0;

   mAttackFactor = exp(-log(_param.mFloor) / (mCurRate * _param.mAttackTime + 0.5));
   mDecayFactor = exp(log(_param.mFloor) / (mCurRate * _param.mDecayTime + 0.5));

   mLastLevel = 0.0;
   
   memset(mCircle,0,sizeof(mCircle));
   memset(follow,0,sizeof(follow));
   memset(mLevelCircle,0,sizeof(mLevelCircle));
    
}
//
AUDMAudioFilterLimiter::~AUDMAudioFilterLimiter()
{
  ADM_info("[DRC] Destroyed\n");
}

uint32_t   AUDMAudioFilterLimiter::fill(uint32_t max,float *output,AUD_Status *status)
{
  uint32_t len,i;
  
  shrink();
  fillIncomingBuffer(status);
  
  
  len=_tail-_head;
  if(len>max) len=max;
  if(len>=DELIM_WINDOW_SIZE) len=DELIM_WINDOW_SIZE-1;
  
  // Count in full sample  i.e. all channels
  len=len-(len%_wavHeader.channels);
  
  // Process..
  if (mLastLevel == 0.0) {
    int preSeed = mCircleSize;
    if (preSeed > len)
      preSeed = len;
    for(i=0; i<preSeed; i++)
      AvgCircle(_incomingBuffer[_head+i]);
  }
  
    for (i = 0; i < len; i++) {
      Follow(_incomingBuffer[_head+i], &follow[i], i);
    }

    for (i = 0; i < len; i++) {
      output[i] =DoCompression(_incomingBuffer[_head+i], follow[i]);
    }  
    _head+=len;
    return len;
}

float AUDMAudioFilterLimiter::AvgCircle(float value)
{
  float level;

   // Calculate current level from root-mean-squared of
   // circular buffer ("RMS")
   mRMSSum -= mCircle[mCirclePos];
   mCircle[mCirclePos] = value*value;
   mRMSSum += mCircle[mCirclePos];
   level = sqrt(mRMSSum/mCircleSize);
   mLevelCircle[mCirclePos] = level;
   mCirclePos = (mCirclePos+1)%mCircleSize;  
   return level;
}

void AUDMAudioFilterLimiter::Follow(float x, float *outEnv, int maxBack)
{
   /*

   "Follow"ing algorithm by Roger B. Dannenberg, taken from
   Nyquist.  His description follows.  -DMM

   Description: this is a sophisticated envelope follower.
    The input is an envelope, e.g. something produced with
    the AVG function. The purpose of this function is to
    generate a smooth envelope that is generally not less
    than the input signal. In other words, we want to "ride"
    the peaks of the signal with a smooth function. The 
    algorithm is as follows: keep a current output value
    (called the "value"). The value is allowed to increase
    by at most rise_factor and decrease by at most fall_factor.
    Therefore, the next value should be between
    value * rise_factor and value * fall_factor. If the input
    is in this range, then the next value is simply the input.
    If the input is less than value * fall_factor, then the
    next value is just value * fall_factor, which will be greater
    than the input signal. If the input is greater than value *
    rise_factor, then we compute a rising envelope that meets
    the input value by working bacwards in time, changing the
    previous values to input / rise_factor, input / rise_factor^2,
    input / rise_factor^3, etc. until this new envelope intersects
    the previously computed values. There is only a limited buffer
    in which we can work backwards, so if the new envelope does not
    intersect the old one, then make yet another pass, this time
    from the oldest buffered value forward, increasing on each 
    sample by rise_factor to produce a maximal envelope. This will 
    still be less than the input.
    
    The value has a lower limit of floor to make sure value has a 
    reasonable positive value from which to begin an attack.
   */

   float level = AvgCircle(x);
   float high = mLastLevel * mAttackFactor;
   float low = mLastLevel * mDecayFactor;

   if (low < _param.mFloor)
      low = _param.mFloor;

   if (level < low)
      *outEnv = low;
   else if (level < high)
      *outEnv = level;
   else {
      // Backtrack
     float attackInverse = 1.0 / mAttackFactor;
     float temp = level * attackInverse;

      int backtrack = 50;
      if (backtrack > maxBack)
         backtrack = maxBack;

      float *ptr = &outEnv[-1];
      int i;
      bool ok = false;
      for(i=0; i<backtrack-2; i++) {
         if (*ptr < temp) {
            *ptr-- = temp;
            temp *= attackInverse;
         }
         else {
            ok = true;
            break;
         }   
      }

      if (!ok && backtrack>1 && (*ptr < temp)) {
         temp = *ptr;
         for (i = 0; i < backtrack-1; i++) {
            ptr++;
            temp *= mAttackFactor;
            *ptr = temp;
         }
      }
      else
         *outEnv = level;
   }

   mLastLevel = *outEnv;
}

float AUDMAudioFilterLimiter::DoCompression(float value, float env)
{
   float mult;
   float out;

   if (env > mThreshold)
      mult = mGain * powf(mThreshold/env, 1.0/_param.mRatio);
   else
      mult = mGain;

   out = value * mult;

   if (out > 1.0)
      out = 1.0;

   if (out < -1.0)
      out = -1.0;

   return out;
}
//EOF
