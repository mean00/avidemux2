/***************************************************************************
    copyright            : (C) 2006 by mean
    email                : fixounet@free.fr
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
#include "math.h"
#include "ADM_Video.h"

#include "ADM_mkv.h"
#include "ADM_a52info.h"
#include "ADM_dcainfo.h"

#if 0
#define vprintf printf
#else
#define vprintf(...) {}
#endif

/**
    \fn mkvAccess
    \brief constructor

*/
mkvAccess::mkvAccess(const char *name,mkvTrak *track)
{
    uint8_t ac3Buffer[20000];
    uint32_t len,sample;
    uint64_t timecode;

   _parser=new ADM_ebml_file();
   ADM_assert(_parser->open(name));
  _track=track;
  ADM_assert(_track);
  _currentBlock=0;
  _currentLace=_maxLace=0;
  goToBlock(0);

  /* In case of AC3, do not trust the header...*/
  if(_track->wavHeader.encoding==WAV_AC3)
  {
     if( getPacket(ac3Buffer, &len, 20000,&timecode))
     {
       uint32_t fq,br,chan,syncoff;
        if( ADM_AC3GetInfo(ac3Buffer, len, &fq, &br, &chan,&syncoff) )
        {
            track->wavHeader.channels=chan;
            track->wavHeader.frequency=fq;
            track->wavHeader.byterate=br;
        }
     }
     goToBlock(0);
  }

  if(_track->wavHeader.encoding==WAV_DTS)
  {
     if( getPacket(ac3Buffer, &len, 20000,&timecode))
     {
       uint32_t syncoff,flags,nbsample;
       ADM_DCA_INFO info;
        if( true==ADM_DCAGetInfo(ac3Buffer, len,&info,&syncoff) )
        {
            track->wavHeader.channels=info.channels;
            track->wavHeader.frequency=info.frequency;
            track->wavHeader.byterate=info.bitrate/8;
        }
     }
     goToBlock(0);
  }


}
/**
    \fn getExtraData
*/
bool      mkvAccess::getExtraData(uint32_t *l, uint8_t **d)
{
    *l=_track->extraDataLen;
    *d=_track->extraData;
    return true;
}
/**
    \fn getDurationInUs
*/
uint64_t  mkvAccess::getDurationInUs(void)
{

    uint32_t limit=_track->index.size();
    if(!limit) return 0;
    return _track->index[limit-1].Dts;
}
/**
    \fn mkvAccess
    \brief destructor
*/
mkvAccess::~mkvAccess()
{
      if(_parser) delete _parser;
      _parser=NULL;
}
/**
    \fn goToCluster
    \brief Change the cluster parser...
*/
uint8_t mkvAccess::goToBlock(uint32_t x)
{
  uint32_t limit=_track->index.size();
  if(x>=limit)
  {
    ADM_warning("Exceeding max cluster : asked: %u max :%u\n",x,limit);
    return 0;  // FIXME
  }

  _parser->seek(_track->index[x].pos);
  _currentLace=_maxLace=0;
  _currentBlock=x;
  return 1;
}
/**
    \fn goToTime
*/
bool      mkvAccess::goToTime(uint64_t timeUs)
{
uint64_t targetUs=timeUs;
int      clus=-1;
    uint32_t limit=_track->index.size();
    if(!limit)
    {
        ADM_warning("No audio index, cannot seek\n");
        return false;
    }
    mkvListOfIndex *dex=&(_track->index);
      // First identify the cluster...

            if(timeUs<(*dex)[0].Dts)
            {
                clus=0;
            }else
            for(int i=0;i<limit-1;i++)
            {
              if(targetUs>=(*dex)[i].Dts && targetUs<(*dex)[i+1].Dts)
              {
                clus=i;
                break;
              }
            }
            if(clus==-1) clus=limit-1; // Hopefully in the last one

            targetUs-=(*dex)[clus].Dts; // now the time is relative
            goToBlock(clus);

            printf("[MKVAUDIO] Asked for %"LLU" us, go to block %d, which starts at %"LLU" ms\n",timeUs,clus,targetUs);
            // Now seek more finely
            // will be off by one frame
#if 0
#define MAX_SEEK_BUFFER 20000
            uint8_t buffer[MAX_SEEK_BUFFER];
            uint32_t len,samples;
            uint64_t timecode;
            while(getPacket(buffer, &len, MAX_SEEK_BUFFER,&timecode))
            {
              uint64_t curTime=_clusters[_currentCluster].Dts;
              vprintf("Wanted: %lu us clusTime : %lu Timecode:%lu us\n",timeUs,_curTimeCodeUs,timecode);
              ADM_assert(len<MAX_SEEK_BUFFER);

              if(timecode>=(timeUs))
              {
                printf("[MKV audio] fine seek to %u \n",timecode);
                return 1;
              }
            }
            printf("Failed to seek to %u mstime\n");
            return 0;
#else
            return 1;
#endif
            return 1;

}
/**
    \fn initLaces
    \brief start a bunch of lace, compute the missing DTSs
*/
bool mkvAccess::initLaces(uint32_t nbLaces,uint64_t time)
{
                _maxLace=nbLaces;
                _currentLace=1; 
                _lastDtsBase=time;
                _currentBlock++;
                if(_currentBlock<_track->index.size()) // is it not the last block
                {
                    uint64_t deltaTime=_track->index[_currentBlock].Dts;
                    deltaTime-=time;
                    _laceIncrementUs=deltaTime/nbLaces;
                    vprintf("***************DeltaTime : %"LLU" inc:%"LLU"\n",deltaTime,_laceIncrementUs);
                } // else keep lastIncrement, which is as good as a random value
                return true;
}
/**
    \fn getPacket
*/
bool    mkvAccess::getPacket(uint8_t *dest, uint32_t *packlen, uint32_t maxSize,uint64_t *timecode)
{
  uint64_t fileSize,len,bsize,pos;
  uint32_t alen,vlen;
  uint64_t id;
  ADM_MKV_TYPE type;
  const char *ss;
  
//  vprintf("Enter: Currently at :%llx\n",_clusterParser->tell());

    // Have we still lace to go ?
    if(_currentLace<_maxLace)
    {
      *packlen= readAndRepeat(dest, _Laces[_currentLace]);
      ADM_assert(*packlen<maxSize);
      vprintf("Continuing lacing : %u bytes, lacing %u/%u\n",*packlen,_currentLace,_maxLace);
      *timecode=_lastDtsBase+_laceIncrementUs*_currentLace;
       vprintf(">>>>>>>>> %"LLU" \n",*timecode);
      _currentLace++;
      return true;
    }
    if(_currentBlock>=_track->index.size()) return false;
    // Else we start a new lace (or no lacing at all)
    goToBlock(_currentBlock);
    mkvIndex *dex=&(_track->index[_currentBlock]);
    uint64_t size=dex->size-3;
    uint64_t time=dex->Dts;
    if(!time && _currentBlock) time=ADM_AUDIO_NO_DTS;
    vprintf("[MKV] Time :%lu block:%u\n",time,_currentBlock);
    // Read headers & flags
     int16_t dummyTime=_parser->readSignedInt(2);
     //if(!track) printf("TC: %d\n",timecode);
     uint8_t flags=_parser->readu8();
     int     lacing=((flags>>1)&3);
        vprintf("[MKV] Lacing : %u\n",lacing);
     *timecode=time;
     switch(lacing)
            {
              case 0 : // no lacing

                      vprintf("No lacing :%d bytes\n",(int)size);
                      *packlen= readAndRepeat(dest,size);              
                      _currentLace=_maxLace=0;
                      _currentBlock++;
                      return 1;
              case 1: //Xiph lacing
                {
                        int nbLaces=_parser->readu8()+1;
                        size--;
                        ADM_assert(nbLaces<MKV_MAX_LACES);
                        for(int i=0;i<nbLaces-1;i++)
                        {
                          int v=0;
                          int lce=0;
                          while(  (v=_parser->readu8())==0xff)
                          {
                                lce+=v;
                                size-=(1+0xff);
                          }
                          lce+=v;
                          size--;
                          size-=v;
                          _Laces[i]=lce;
                        }

                        // The first one has Dts
                        *packlen= readAndRepeat(dest, _Laces[0]);              
                        _Laces[nbLaces-1]=size;

                        initLaces(nbLaces,time);
                        return 1;
                      }

                      break;
              case 2 : // constant size lacing
                      {
                        int nbLaces=_parser->readu8()+1;
                        size--;
                        int bsize=size/nbLaces;
                        vprintf("NbLaces :%u lacesize:%u\n",nbLaces,bsize);
                        ADM_assert(nbLaces<MKV_MAX_LACES);
                        for(int i=0;i<nbLaces;i++)
                        {
                          _Laces[i]=bsize;
                        }
                        *packlen= readAndRepeat(dest, bsize);              
                        // The first one has Dts
                        initLaces(nbLaces,time);
                       
                        return 1;
                      }
                      break;

              case 3: // Ebml lacing
                {
                        uint64_t head=_parser->tell();
                        int nbLaces=_parser->readu8()+1;
                        int32_t curSize=_parser->readEBMCode();
                        int32_t delta;
                        uint32_t sum;


                        vprintf("Ebml nbLaces :%u lacesize(0):%u\n",nbLaces,curSize);

                        _Laces[0]=curSize;
                        sum=curSize;
                        ADM_assert(nbLaces<MKV_MAX_LACES);
                        for(int i=1;i<nbLaces-1;i++)
                        {
                          delta=_parser->readEBMCode_Signed();
                          vprintf("Ebml delta :%d lacesize[%d]->:%d\n",delta,i,curSize+delta);
                          curSize+=delta;
                          ADM_assert(curSize>0);
                          _Laces[i]=curSize;
                          sum+=curSize;

                        }
                        uint64_t tail=_parser->tell();
                        uint64_t consumed=head+size-tail;

                        _Laces[nbLaces-1]=consumed-sum;

                          // Take the 1st laces, it has timestamp
                          *packlen= readAndRepeat(dest, _Laces[0]);              
                          ADM_assert(*packlen<maxSize);
                          vprintf("Continuing lacing : dts : %lu %u bytes, lacing %u/%u\n",time,*packlen,_currentLace,_maxLace);
                          initLaces(nbLaces,time);
                          return 1;
                }
                      break;
              default:
                    printf("Unsupported lacing %u\n",lacing);
                    goToBlock(0);
            }

  return false;
}
//EOF
