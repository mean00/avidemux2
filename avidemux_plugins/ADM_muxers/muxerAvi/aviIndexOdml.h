/***************************************************************************
                 \file         aviIndexOdml.h
                 \brief        Write odml / avi type2 index/indeces
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
#include "aviIndex.h"
#include "aviIndexAvi.h" // needed to convert type1 to type2
#define AVI_INDEX_CHUNK_SIZE (16*1024)

#define AVI_INDEX_SUPERINDEX 0
#define AVI_INDEX_INDEX      1
//-------------------------- super index -----------------------
/**
     \struct odmlSuperIndex
*/
typedef struct 
{
            uint64_t offset;
            uint32_t size;
            uint32_t duration;
}odmlIndecesDesc;
/**
    \class odmlOneSuperIndex
*/
class odmlOneSuperIndex
{
public:
            uint32_t   fcc;
            std::vector <odmlIndecesDesc> indeces;
            void        serialize(AviListAvi *list);
};
/**
    \fn odmlSuperIndex
*/
class odmlSuperIndex
{
public:
            odmlOneSuperIndex trackIndeces[1+ADM_AVI_MAX_AUDIO_TRACK];
public:
                        odmlSuperIndex() {};
            

}; // followed by [] of uint64_t offset/uint32_t size/uint32_t duration

//-------------------------- regular index -----------------------
/**
    \class odmlIndexChunk
*/
typedef struct 
{
    uint32_t offset;
    uint32_t size;
    uint32_t flags;
}odmIndexEntry;
class odmlRegularIndex
{
public:
        uint64_t   baseOffset;
        uint64_t   indexPosition;
        std::vector <odmIndexEntry> listOfChunks;
        bool        serialize(AviListAvi *list,uint32_t fccTag,int trackNumber);
};
//------------------------------------------------------------
/**
    \class aviIndexBase
*/
class aviIndexOdml : public aviIndexBase
{
protected:
           bool             writeSuperIndex();
           bool             writeRegularIndex(int trackNumber);
           odmlSuperIndex   superIndex;
           odmlRegularIndex indexes[1+ADM_AVI_MAX_AUDIO_TRACK];
           bool             startNewRiffIfNeeded(int trackNo,int len);
           bool             startNewRiff();

public:
                        aviIndexOdml(aviWrite *father,aviIndexAvi *cousin );
                        aviIndexOdml(aviWrite *father,AviListAvi *lst) ;
           virtual      ~aviIndexOdml();
           virtual bool  addVideoFrame( int len,uint32_t flags,const uint8_t *data);
           virtual bool  addAudioFrame(int trackNo, int len,uint32_t flags,const uint8_t *data);
           virtual bool  writeIndex();
           virtual int   getNbVideoFrameForHeaders();
};