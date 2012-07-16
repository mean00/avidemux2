                
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
aviIndexOdml::aviIndexOdml(aviWrite *father,aviIndexAvi *cousin,uint64_t odmlPos )
    : aviIndexBase(father,cousin->_masterList,odmlPos)  
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
aviIndexOdml::aviIndexOdml(aviWrite *father,AviListAvi *lst,uint64_t odmlChunk ): aviIndexBase(father,lst,odmlChunk)  
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
    riffCount=0;
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
        supEntry.size=AVI_REGULAR_INDEX_CHUNK_SIZE;
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
       list.fill(AVI_SUPER_INDEX_CHUNK_SIZE);
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
       list.fill(AVI_REGULAR_INDEX_CHUNK_SIZE);
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
        LMovie->writeDummyChunk(AVI_REGULAR_INDEX_CHUNK_SIZE,&pos);
        indexes[0].indexPosition=pos;
        odmIndexEntry ix;
        ix.flags=flags;
        ix.offset=indexes[0].baseOffset;
        ix.size=len;
        indexes[0].listOfChunks.push_back(ix);
        nbVideoFrame++;
        return true;
    }
        startNewRiffIfNeeded(0,len);
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
            LMovie->writeDummyChunk(AVI_REGULAR_INDEX_CHUNK_SIZE,&pos);
            thisIndex->indexPosition=pos;
            odmIndexEntry ix;
            ix.flags=flags;
            ix.offset=thisIndex->baseOffset;
            ix.size=len;
            thisIndex->listOfChunks.push_back(ix);
            audioFrameCount[trackNo]++;
            audioSizeCount[trackNo]+=len;
            return true;
        }
        //
        //  Check if we need a new index...
        startNewRiffIfNeeded(1+trackNo,len);
        odmIndexEntry ix;
        ix.flags=flags;
        ix.offset=LMovie->Tell();
        ix.size=len;
        thisIndex->listOfChunks.push_back(ix);

        LMovie->WriteChunk(fourccs[1+trackNo],len,data);
        audioFrameCount[trackNo]++;
        audioSizeCount[trackNo]+=len;
        return true;
}
/**
    \fn writeOdmlChunk
*/

bool aviIndexOdml::writeOdmlChunk()
{
            uint64_t off=LMovie->Tell();

            LMovie->Seek(odmlChunkPosition);
            AviListAvi dum("LIST",LMovie->getFile());
            dum.Begin();
            dum.Write32("odml");
            dum.Write32("dmlh");
            dum.Write32(4);
            dum.Write32(nbVideoFrame); // Real number of video frames
            dum.EndAndPaddTilleSizeMatches(248+12);
            


            LMovie->Seek(off);
            return true;
}

/**
    \fn writeIndex
*/

bool  aviIndexOdml::writeIndex()
{
            // super index needed ?
            ADM_info("Writting openDml chunk\n");
            writeOdmlChunk();
            ADM_info("Writting type 2 Avi index\n");
            for(int i=0;i<1+nbAudioTrack;i++)
                writeRegularIndex(i);
            ADM_info("Writting type 2 Avi SuperIndex\n");
            writeSuperIndex();
            LMovie->End();
            if(!riffCount)
            {
                ADM_info("Writting legacy index\n");
                writeLegacyIndex();
            }

            delete LMovie;  
            LMovie=NULL;

            _masterList->End();
            delete _masterList;
            _masterList=NULL;
            return true;
}
/**
    \fn startNewRiffIfNeeded
*/
bool aviIndexOdml::startNewRiffIfNeeded(int trackNo,int len)
{
    bool breakNeeded=false;

    // Case 1: we exceed riff boundary (4 GB)
        uint64_t currentPosition=LMovie->Tell();
        uint64_t start=_masterList->TellBegin();
        uint64_t riffSize=currentPosition-start;
        uint64_t limit=((1LL<<31)-10*(1LL<<20)); // 2GB per riff chunk
        riffSize+=len;
        if(!riffCount) // take into account legacy index
        {
            for(int i=0;i<1+nbAudioTrack;i++)
                limit-=4*4*indexes[i].listOfChunks.size();
        }
        if(riffSize> limit)
        {
            ADM_info("Riff is now %"LLU" bytes, break needed\n",riffSize);
            breakNeeded=true;
        }

    // Case 2 : the current index is full
        int available=(AVI_REGULAR_INDEX_CHUNK_SIZE-64)/8; // nb index entry
        if(indexes[trackNo].listOfChunks.size()>=available)
        {
            ADM_info("Index for track %d is full\n",trackNo);
            breakNeeded=true;
        }
        if(breakNeeded)
            startNewRiff();
        return true;
}
/**
    \fn startNewRiffIfNeeded
*/
bool aviIndexOdml::startNewRiff()
{
    uint64_t pos;
    pos=LMovie->Tell();
    ADM_info("Starting new riff at position %"LLU" (0x%"LLX")",pos,pos);

    // 0- Write legacy index, else WMP is not happy...

    // 1- Write all pending regular index
    for(int i=0;i<nbAudioTrack+1;i++)
        writeRegularIndex(i);

    // 2- Start a new RIFF
    LMovie->End();          // Close current riff
    if(!riffCount)
        writeLegacyIndex();

    _masterList->End();
    _masterList->Begin();
    _masterList->Write32("AVIX");
    LMovie->Begin();
    LMovie->Write32("movi");
    // 3- Write placeHolder for odml index
    for(int i=0;i<nbAudioTrack+1;i++)
    {
             
             LMovie->writeDummyChunk(AVI_REGULAR_INDEX_CHUNK_SIZE,&pos);
             indexes[i].baseOffset=indexes[i].indexPosition=pos;
    }
    riffCount++;    
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
/**
    \fn writeLegacyIndex
*/
bool aviIndexOdml::writeLegacyIndex()
{
    uint64_t pos;
    pos=LMovie->Tell();
    ADM_info("Writting legacy index at %"LLX"\n",pos);
    //---
    AviListAvi lst("idx1",LMovie->getFile());
    lst.Begin();
    int *idx=new int[1+nbAudioTrack];
    for(int i=0;i<1+nbAudioTrack;i++) idx[i]=0;


    delete []idx;
    lst.End();
    //--
   // LMovie->Seek(pos);
    return true;
}
// EOF