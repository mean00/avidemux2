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
class aviIndexOdml;
/**
    \class aviIndexBase
*/
class aviIndexBase
{

public:
            uint64_t         superIndexPosition[ADM_AVI_MAX_AUDIO_TRACK+1];
protected:
            uint32_t         fourccs[ADM_AVI_MAX_AUDIO_TRACK+1];
            uint64_t         currentBaseOffset;
            AviListAvi	      *LMovie ;
            AviListAvi        *_masterList;
            int              nbVideoFrame;
            int              nbAudioTrack;
            int              audioFrameCount[ADM_AVI_MAX_AUDIO_TRACK];
            int              audioSizeCount[ADM_AVI_MAX_AUDIO_TRACK];
            uint64_t        openDmlHeaderPosition[1+ADM_AVI_MAX_AUDIO_TRACK];
            uint64_t        odmlChunkPosition;

public:
                                aviIndexBase(aviWrite *father,AviListAvi *lst,uint64_t odmlChunkPosition) ;
                            
           virtual              ~aviIndexBase() {};
           virtual bool         addVideoFrame( int len,uint32_t flags,const uint8_t *data)=0;
           virtual bool         addAudioFrame(int trackNo, int len,uint32_t flags,const uint8_t *data)=0;
           virtual bool         writeIndex()=0;
           virtual int          getNbVideoFrameForHeaders()=0;
                    uint32_t    getSizeInBytesForAudioTrack(int i){ return audioSizeCount[i];}
                    

};
//EOF
