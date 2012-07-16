                
/** *************************************************************************
        \file  aviIndexAvi
        \brief write type1 avi with only one index

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
#include "aviIndexAvi.h"
#include "op_aviwrite.hxx"
#include "fourcc.h"
/**
    \fn ctor
*/
aviIndexAvi::aviIndexAvi(aviWrite *father,AviListAvi *lst,uint64_t odmlChunk ): aviIndexBase(father,lst,odmlChunk)  
{
  		
    LMovie = new AviListAvi ("LIST", father->_file);
    LMovie->Begin();
    LMovie->Write32("movi");
}
/**
    \fn dtor
*/

aviIndexAvi::~aviIndexAvi() 
{
    if(LMovie)
        delete LMovie;
    LMovie=NULL;
}
/**
    \fn addVideoFrame
*/
bool  aviIndexAvi::addVideoFrame(int len,uint32_t flags,const uint8_t *data)
{
      IdxEntry entry;
      uint64_t offset=LMovie->Tell () - 8 - LMovie->TellBegin ();
      entry.fcc = fourccs[0];
      entry.len = len;
      entry.flags = flags;
      entry.offset = offset;
      myIndex.push_back(entry);
    
      LMovie->WriteChunk (entry.fcc, len, data);
      return true;
}
/**
    \fn addAudioFrame
*/

bool  aviIndexAvi::addAudioFrame(int trackNo,int len,uint32_t flags,const uint8_t *data)
{
 IdxEntry entry;
    uint64_t offset=LMovie->Tell () - 8 - LMovie->TellBegin ();
      entry.fcc = entry.fcc = fourccs[trackNo+1];
      entry.len = len;
      entry.flags = flags;
      entry.offset = offset;
      myIndex.push_back(entry);
      LMovie->WriteChunk (entry.fcc, len, data);
      audioSizeCount[trackNo]+=len;
      audioFrameCount[trackNo]++;      
      return true;
}
/**
    \fn writeIndex
    \brief write the final index if needed
*/

bool  aviIndexAvi::writeIndex()
{
    // End the movie atom
  LMovie->End ();
  delete LMovie;
  LMovie = NULL;
    // and start the idx1  = avi type1 index

    #define ix32(a)  memIo.write32(myIndex[i].a)
    ADM_info("Writing type 1 Avi index\n");
    // Write index
    int curindex=myIndex.size();
    AviListAvi *lst=new AviListAvi("idx1",_masterList->getFile());
    ADMMemio memIo(4*4);
    for (uint32_t i = 0; i < curindex; i++)
    {
        memIo.reset();
        ix32(fcc);ix32(flags);ix32(offset);ix32(len);
        lst->WriteMem (memIo);
    }
    delete lst;
    lst=NULL;
    _masterList->End();
    delete _masterList;
    _masterList=NULL;
    ADM_info("Done writing type 1 Avi index\n");
    return true;
}
/**
    \fn getNbVideoFrameForHeaders
    \brief for type1 avi, just return the actual # of frames
*/
int   aviIndexAvi::getNbVideoFrameForHeaders()
{
        return nbVideoFrame;
}

//-------------------------------------------------
/**
    \fn ctor
    \brief constructor for base class
*/
aviIndexBase::aviIndexBase(aviWrite *father,AviListAvi *lst,uint64_t odmlChunk)
{
      odmlChunkPosition=odmlChunk;
      _masterList=lst;
      nbVideoFrame=0;   
      memset(audioFrameCount,0,sizeof(audioFrameCount));
      memset(audioSizeCount,0,sizeof(audioSizeCount));
      nbAudioTrack=father->nb_audio;
      currentBaseOffset=0;
      fourccs[0]=fourCC::get ((uint8_t *)"00dc");
        for(int i=0;i<ADM_AVI_MAX_AUDIO_TRACK;i++)
        {
            char txt[10]="01wd";
            txt[1]+=i;
            fourccs[i+1]=fourCC::get (( uint8_t *)txt);
        }
        for(int i=0;i<1+ADM_AVI_MAX_AUDIO_TRACK;i++)       
               openDmlHeaderPosition[i]=father->openDmlHeaderPosition[i];

}
// EOF