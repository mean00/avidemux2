                
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
    LMovie->Begin();
    LMovie->Write32("movi");
    nbVideoFrame=0;
    memset(audioFrameCount,0,sizeof(audioFrameCount));
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
    
    for(int i=0;i<nb;i++)
    {
        odmlOneSuperIndex *cur=superIndex.trackIndeces+i; 
        uint64_t pos=_father->openDmlHeaderPosition[i];
        ADM_info("Writing  superIndex %d at %"LLX"\n",i,pos);
        LMovie->Seek(pos);       
        cur->serialize(LMovie);
    }
    LMovie->Seek(off); // rewind
    return true;
}
/**
    \
*/
bool           aviIndexOdml::writeRegularIndex(int trackNumber)
{
    uint64_t off=LMovie->Tell();
    aprintf("Writing index part for track %d\n",trackNumber);
    odmlRegularIndex *cur=indexes+trackNumber;
    if(cur->listOfChunks.size())
    {
        // Write index
        ADM_info("Writing regular index for track %d, at position 0x%"LLX"\n",trackNumber,cur->indexPosition);
        LMovie->Seek(cur->indexPosition);
        cur->serialize(LMovie,fourccs[trackNumber],trackNumber);
        cur->listOfChunks.clear();
        LMovie->Seek(off); // rewind
        // add to super index
        odmlOneSuperIndex *super=superIndex.trackIndeces+trackNumber;
        odmlIndecesDesc supEntry;
        supEntry.offset=cur->indexPosition;
        supEntry.size=AVI_INDEX_CHUNK_SIZE;
        supEntry.duration=0; // Fixme
        super->indeces.push_back(supEntry);
    }
    
    return true;
}
/**
    \fn serialize
    \brief write the super index for a given track...
*/
void        odmlOneSuperIndex::serialize(AviListAvi *parentList)
{
       ADMFile *f=parentList->getFile() ;       
       AviListAvi list("indx",f);
       list.Begin();
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
       list.End();

}
/**
    \fn serialize
    \brief write one index entry
*/
bool        odmlRegularIndex::serialize(AviListAvi *parentList,uint32_t fccTag,int trackNumber)
{
       ADMFile *f=parentList->getFile() ;
       char fcc[5]="ix00";
       fcc[3]+=trackNumber;
       AviListAvi list(fcc,f);
       list.Begin();
       list.Write32(2); // Size of each entry in dword
       list.Write8(0);
       list.Write8(AVI_INDEX_INDEX);
       int n=listOfChunks.size();
       list.Write32(n); // Entries in use

       list.Write32(fccTag);            //chunkId fourccs[trackNumber]);

       list.Write64(baseOffset);                  // base offset
       list.Write32((uint32_t)0);      // reserved3
       for(int i=0;i<n;i++)
       {
            odmIndexEntry ix=listOfChunks[i];
            if(ix.offset<baseOffset)
            {
                    ADM_warning("Fatal error : Chunk is at %"LLU" but base is at %"LLU"\n",ix.offset,baseOffset);
                    exit(-1);
            }
            
            list.Write32(ix.offset-baseOffset);
            if(ix.flags & AVI_KEY_FRAME)
                list.Write32(ix.size);
            else
                list.Write32(0x80000000LL+ix.size);
       }
       list.fill(AVI_INDEX_CHUNK_SIZE);
       list.End();
       return true;
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
        indexes[0].indexPosition=pos;
        odmIndexEntry ix;
        ix.flags=flags;
        ix.offset=0;
        ix.size=len;
        indexes[0].listOfChunks.push_back(ix);
        nbVideoFrame++;
        return true;
    }
     //  Check if we need a new index...
        int available=(AVI_INDEX_CHUNK_SIZE-64)/8; // nb index entry
        if(indexes[0].listOfChunks.size()>=available)
        {
            // 1- Flush old index
             writeRegularIndex(0);
             uint64_t pos;
            // 2- Create  new index
             LMovie->writeDummyChunk(AVI_INDEX_CHUNK_SIZE,&pos);
             indexes[0].baseOffset=indexes[0].indexPosition=pos;
        }
        odmIndexEntry ix;
        ix.flags=flags;
        ix.offset=LMovie->Tell();
        ix.size=len;
        indexes[0].listOfChunks.push_back(ix);

     
        LMovie->WriteChunk(fourccs[0],len,data);
        
        nbVideoFrame++;
        return true;
}
/**
    \fn addAudioFrame
*/

bool  aviIndexOdml::addAudioFrame(int trackNo,int len,uint32_t flags,const uint8_t *data)
{
        // check riff break
        if(!audioFrameCount[trackNo])
        {

            indexes[1+trackNo].baseOffset=LMovie->Tell();
            // Write video frame
            LMovie->WriteChunk(fourccs[1+trackNo],len,data);
            // Create place holder for index
            uint64_t pos;
            LMovie->writeDummyChunk(AVI_INDEX_CHUNK_SIZE,&pos);
            indexes[1+trackNo].indexPosition=pos;
            odmIndexEntry ix;
            ix.flags=flags;
            ix.offset=0;
            ix.size=len;
            indexes[1+trackNo].listOfChunks.push_back(ix);
            audioFrameCount[trackNo]++;
            return true;
        }
        //
        //  Check if we need a new index...
        int available=(AVI_INDEX_CHUNK_SIZE-64)/8; // nb index entry
        if(indexes[1+trackNo].listOfChunks.size()>=available)
        {
            // 1- Flush old index
             writeRegularIndex(1+trackNo);
             uint64_t pos;
            // 2- Create  new index
             LMovie->writeDummyChunk(AVI_INDEX_CHUNK_SIZE,&pos);
             indexes[1+trackNo].indexPosition=indexes[1+trackNo].baseOffset=pos;
        }
        odmIndexEntry ix;
        ix.flags=flags;
        ix.offset=LMovie->Tell();
        ix.size=len;
        indexes[1+trackNo].listOfChunks.push_back(ix);

        LMovie->WriteChunk(fourccs[1+trackNo],len,data);
        audioFrameCount[trackNo]++;
        return true;
}
/**
    \fn writeIndex
*/

bool  aviIndexOdml::writeIndex()
{
            ADM_info("Writing type 2 Avi index\n");
            for(int i=0;i<1+_father->nb_audio;i++)
                writeRegularIndex(i);
            ADM_info("Writing type 2 Avi SuperIndex\n");
            writeSuperIndex();
            return true;
}

// EOF