/***************************************************************************
                 \file         aviIndex.h
                 \brief        Virtual base class for index management for avi
                 \author       mean fixounet@free.Fr (c) 2012
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#pragma once
class aviWrite;
/**
    \class aviIndexBase
*/
#define ADM_AVI_MAX_AUDIO_TRACK 5 // Fixme : dupe
#include "fourcc.h"
#include "avilist_avi.h"
/**
    \class aviIndexBase
*/
class aviIndexBase
{
public:
            uint64_t         superIndexPosition[ADM_AVI_MAX_AUDIO_TRACK+1];
protected:
            uint32_t         fourccs[ADM_AVI_MAX_AUDIO_TRACK+1];
            aviWrite          *_father;
            uint64_t         currentBaseOffset;
            AviListAvi	      *LMovie ;
public:
                        aviIndexBase(aviWrite *father) 
                            {
                                  _father=father;
                                  currentBaseOffset=0;
                                  fourccs[0]=fourCC::get ((uint8_t *)"00dc");
                                    for(int i=0;i<ADM_AVI_MAX_AUDIO_TRACK;i++)
                                    {
                                        char txt[10]="01wd";
                                        txt[2]+=i;
                                        fourccs[i+1]=fourCC::get (( uint8_t *)txt);
                                    }

                            };
           virtual      ~aviIndexBase() {};
           virtual bool  addVideoFrame( int len,uint32_t flags,const uint8_t *data)=0;
           virtual bool  addAudioFrame(int trackNo, int len,uint32_t flags,const uint8_t *data)=0;
           virtual bool  writeIndex()=0;

};