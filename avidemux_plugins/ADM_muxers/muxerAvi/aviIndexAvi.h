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
#include "aviIndex.h"
#include "avilist_avi.h"
typedef struct
{
  uint32_t fcc;
  uint32_t flags;
  uint32_t offset;
  uint32_t len;
}IdxEntry;
class aviIndexOdml;
/**
    \class aviIndexBase
*/
class aviIndexAvi : public aviIndexBase
{
friend class aviIndexOdml;
protected:
           
           std::vector <IdxEntry > myIndex;
           

public:
                        aviIndexAvi(aviWrite *father,AviListAvi *lst,uint64_t odmlChunk) ;
           virtual      ~aviIndexAvi();
           virtual bool  addVideoFrame( int len,uint32_t flags,const uint8_t *data);
           virtual bool  addAudioFrame(int trackNo, int len,uint32_t flags,const uint8_t *data);
           virtual bool  writeIndex();
           virtual int   getNbVideoFrameForHeaders();

};