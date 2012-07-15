                
/** *************************************************************************
        \file op_aviwrite.cpp
        \brief low level avi muxer

		etc...


    copyright            : (C) 2002-2012 by mean
                           (C) Feb 2005 by GMV: ODML write support
    GPL V2.0
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "ADM_default.h"
#include "vector"
#include "aviIndexOdml.h"
#include "op_aviwrite.hxx"
#include "fourcc.h"
#define aprintf(...) {}

/**
    \fn ctor
*/
aviIndexOdml::aviIndexOdml(aviWrite *father ): aviIndexBase(father)  
{
    LMovie = new AviListAvi ("LIST", father->_file);
    LMovie->Begin ("movi");
    nbVideoFrame=0;
}
/**
    \fn dtor
*/

aviIndexOdml::~aviIndexOdml() 
{
    if(LMovie)
        delete LMovie;
    LMovie=NULL;
}

/**
    \fn writeSuperIndex
*/
bool aviIndexOdml::writeSuperIndex()
{
    int nb= 1+_father->nb_audio;
    uint64_t off=LMovie->Tell();
    ADM_info("Writing %d superIndex\n",nb);
    for(int i=0;i<nb;i++)
    {
        odmlOneSuperIndex *cur=superIndex.trackIndeces+i;        
        cur->serialize(LMovie);
    }
    LMovie->Seek(off); // rewind
    return true;
}
/**
    \fn serialize
    \brief write the super index for a given track...
*/
void        odmlOneSuperIndex::serialize(AviListAvi *parentList)
{
       parentList->Seek(superIndexPosition); // go to super index position
       ADMFile *f=parentList->getFile() ;
       AviListAvi list("indx",f);
       list.Write32(4); // Size of each entry
       list.Write8(0);
       list.Write8(AVI_INDEX_SUPERINDEX);
       int n=indeces.size();
       list.Write32(n); // Entries in use
       list.Write32(fcc);
       list.Write32((uint32_t)0);
       list.Write32((uint32_t)0);
       list.Write32((uint32_t)0);
       for(int i=0;i<n;i++)
       {
            odmlIndecesDesc ix=indeces[i];
            list.Write64(ix.offset);
            list.Write32(ix.size);
            list.Write32(ix.duration);
       }
       // write filler...
       list.fill(AVI_INDEX_CHUNK_SIZE);
}
/**
    \fn serialize
    \brief write one index entry
*/
bool        odmlRegularIndex::serialize(AviListAvi *parentList)
{
       ADMFile *f=parentList->getFile() ;
       char fcc[5]="ix00";
       fcc[3]+=trackNumber;
       AviListAvi list(fcc,f);
       list.Write32(2); // Size of each entry in dword
       list.Write8(0);
       list.Write8(AVI_INDEX_INDEX);
       int n=listOfChunks.size();
       list.Write32(n); // Entries in use
       list.Write32(fcc);
       list.Write32((uint32_t)0);
       list.Write32((uint32_t)0);
       list.Write32((uint32_t)0);
       for(int i=0;i<n;i++)
       {
            odmIndexEntry ix=listOfChunks[i];
            list.Write32(ix.offset);
            list.Write32(ix.size);
       }
       list.fill(AVI_INDEX_CHUNK_SIZE);
       // write filler...
}

/**
    \fn addVideoFrame
*/
bool  aviIndexOdml::addVideoFrame(int len,uint32_t flags,const uint8_t *data)
{
    // our very first frame ?
    if(!nbVideoFrame)
    {
        indexes[0].baseOffset=LMovie->Tell();
        // Write video frame
        LMovie->WriteChunk(fourccs[0],len,data);
        // Create place holder for index
        uint64_t pos;
        LMovie->writeDummyChunk(AVI_INDEX_CHUNK_SIZE,&pos);
        odmIndexEntry ix;
        ix.flags=flags;
        ix.offset=0;
        ix.size=len;
        indexes[0].listOfChunks.push_back(ix);
        nbVideoFrame++;
        return true;
    }
     // check riff break
        LMovie->WriteChunk(fourccs[0],len,data);
        odmIndexEntry ix;
        ix.flags=flags;
        ix.offset=0;
        ix.size=len;
        indexes[0].listOfChunks.push_back(ix);
        nbVideoFrame++;
        return true;
}
/**
    \fn addAudioFrame
*/

bool  aviIndexOdml::addAudioFrame(int trackNo,int len,uint32_t flags,const uint8_t *data)
{
 // check riff break
        LMovie->WriteChunk(fourccs[1+trackNo],len,data);
        odmIndexEntry ix;
        ix.flags=flags;
        ix.offset=0;
        ix.size=len;
        indexes[1+trackNo].listOfChunks.push_back(ix);
        return true;
}
/**
    \fn writeIndex
*/

bool  aviIndexOdml::writeIndex()
{
            ADM_info("Writing type 2 Avi index\n");
            writeSuperIndex();
            return true;
}

// EOF