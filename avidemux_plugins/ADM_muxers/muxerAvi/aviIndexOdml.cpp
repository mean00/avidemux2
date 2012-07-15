                
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
#if 1
#define aprintf(...) {}
#else
#define aprintf printf
#endif
/**
    \fn ctor
    \brief this one is used when converting a type 1 avi to type 2
*/
aviIndexOdml::aviIndexOdml(aviWrite *father,aviIndexAvi *cousin ): aviIndexBase(father,cousin->_masterList)  
{
    ADM_info("Creating Odml file from avi/type1... \n");
    LMovie = cousin->LMovie; // steal movie from cousin
    cousin->LMovie=NULL;
    nbVideoFrame=cousin->nbVideoFrame;
    memset(audioFrameCount,0,sizeof(audioFrameCount));
    for(int i=0;i<ADM_AVI_MAX_AUDIO_TRACK;i++)
        audioFrameCount[i]=cousin->audioFrameCount[i];
    // Prepare fcc for superIndex
    superIndex.trackIndeces[0].fcc=fourCC::get((uint8_t *)"00dc");
    for(int i=0;i<ADM_AVI_MAX_AUDIO_TRACK;i++)
    {
        char txt[5]="01wb";
        txt[1]+=i;
        superIndex.trackIndeces[1+i].fcc=fourCC::get((uint8_t *)txt);
    }
    // Convert cousin's index
    int n=cousin->myIndex.size();
    for(int j=0;j<ADM_AVI_MAX_AUDIO_TRACK+1;j++)
    {
        uint32_t trackFcc=superIndex.trackIndeces[j].fcc;
        for(int i=0;i<n;i++)
        {
            IdxEntry trx=cousin->myIndex[i];
            // 
            if(trx.fcc==trackFcc)
            {
                    odmIndexEntry ix;
                    ix.flags=trx.flags;
                    ix.offset=trx.offset;
                    ix.size=trx.len;
                    indexes[j].listOfChunks.push_back(ix);
            }
        }
    }
    cousin->myIndex.clear(); // empty cousin index
    for(int j=0;j<ADM_AVI_MAX_AUDIO_TRACK+1;j++)
        printf("Track %d, found %d entries\n",j,(int)indexes[j].listOfChunks.size());
}

/**
    \fn ctor
*/
aviIndexOdml::aviIndexOdml(aviWrite *father,AviListAvi *lst ): aviIndexBase(father,lst)  
{
    LMovie = new AviListAvi ("LIST", father->_file);
    LMovie->Begin();
    LMovie->Write32("movi");
    // Prepare fcc for superIndex
    superIndex.trackIndeces[0].fcc=fourCC::get((uint8_t *)"00dc");
    for(int i=0;i<ADM_AVI_MAX_AUDIO_TRACK;i++)
    {
        char txt[5]="01wb";
        txt[1]+=i;
        superIndex.trackIndeces[1+i].fcc=fourCC::get((uint8_t *)txt);
    }
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
    int nb= 1+nbAudioTrack;
    uint64_t off=LMovie->Tell();
    
    for(int i=0;i<nb;i++)
    {
        odmlOneSuperIndex *cur=superIndex.trackIndeces+i; 
        uint64_t pos=openDmlHeaderPosition[i];
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
    int nbEntries=cur->listOfChunks.size();
    if(nbEntries)
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
        supEntry.duration=nbEntries; // Fixme ??
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
       list.Write16(4); // Size of each entry
       list.Write8(0);
       list.Write8(AVI_INDEX_SUPERINDEX);
       int n=indeces.size();
       list.Write32(n); // Entries in use
       list.Write32(fcc);
       list.Write32((uint32_t)0);
       list.Write32((uint32_t)0);
       list.Write32((uint32_t)0);
       aprintf("We have %d entries in that superIndex\n",n);
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
       list.Write16(2); // Size of each entry in dword
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
            if(ix.offset+8<baseOffset)
            {
                    ADM_warning("Fatal error : Chunk is at %"LLU" but base is at %"LLU"\n",ix.offset,baseOffset);
                    exit(-1);
            }
            
            list.Write32(ix.offset+8-baseOffset);
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
        ix.offset=indexes[0].baseOffset;
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
        odmlRegularIndex *thisIndex=indexes+1+trackNo;
        // check riff break
        if(!audioFrameCount[trackNo])
        {

            thisIndex->baseOffset=LMovie->Tell();
            // Write video frame
            LMovie->WriteChunk(fourccs[1+trackNo],len,data);
            // Create place holder for index
            uint64_t pos;
            LMovie->writeDummyChunk(AVI_INDEX_CHUNK_SIZE,&pos);
            thisIndex->indexPosition=pos;
            odmIndexEntry ix;
            ix.flags=flags;
            ix.offset=thisIndex->baseOffset;
            ix.size=len;
            thisIndex->listOfChunks.push_back(ix);
            audioFrameCount[trackNo]++;
            return true;
        }
        //
        //  Check if we need a new index...
        int available=(AVI_INDEX_CHUNK_SIZE-64)/8; // nb index entry
        if(thisIndex->listOfChunks.size()>=available)
        {
            // 1- Flush old index
             writeRegularIndex(1+trackNo);
             uint64_t pos;
            // 2- Create  new index
             LMovie->writeDummyChunk(AVI_INDEX_CHUNK_SIZE,&pos);
             thisIndex->indexPosition=thisIndex->baseOffset=pos;
        }
        odmIndexEntry ix;
        ix.flags=flags;
        ix.offset=LMovie->Tell();
        ix.size=len;
        thisIndex->listOfChunks.push_back(ix);

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
            for(int i=0;i<1+nbAudioTrack;i++)
                writeRegularIndex(i);
            ADM_info("Writing type 2 Avi SuperIndex\n");
            writeSuperIndex();
            LMovie->End();
            delete LMovie;  
            LMovie=NULL;

            _masterList->End();
            delete _masterList;
            _masterList=NULL;
            return true;
}
/**
    \fn getNbVideoFrameForHeaders
    \brief for type1 avi, just return the actual # of frames
*/
int   aviIndexOdml::getNbVideoFrameForHeaders()
{
        return superIndex.trackIndeces[0].indeces.size();
}

// EOF