/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#ifndef LIMITER_PARAM_H
#define LIMITER_PARAM_H
#define DRC_WINDOW 100
typedef struct 
{
  uint32_t mUseGain;
  double   mFloor;
  double   mAttackTime;
  double   mDecayTime;
  double   mRatio;
  double   mThresholdDB;
   // double   mGainDB;  
}DRCparam;

#endif
