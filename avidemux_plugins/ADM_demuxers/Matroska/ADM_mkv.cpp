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
#include <math.h>

#include "ADM_default.h"
#include "ADM_Video.h"
#include "ADM_mkv.h"
#include "mkv_tags.h"
/**
    \fn open
    \brief Try to open the mkv file given as parameter
    
*/

uint8_t mkvHeader::open(const char *name)
{

  ADM_ebml_file ebml;
  uint64_t id,len;
  uint64_t alen;
  ADM_MKV_TYPE type;
  const char *ss;


  _isvideopresent=0;
  if(!ebml.open(name))
  {
    printf("[MKV]Failed to open file\n");
    return 0;
  }
  if(!ebml.find(ADM_MKV_PRIMARY,EBML_HEADER,(MKV_ELEM_ID)0,&alen))
  {
    printf("[MKV] Cannot find header\n");
    return 0;
  }
  if(!checkHeader(&ebml,alen))
  {
     printf("[MKV] Incorrect Header\n");
     return 0;
  }

  /* Now find tracks */
  if(!ebml.find(ADM_MKV_SECONDARY,MKV_SEGMENT,MKV_TRACKS,&alen))
  {
     printf("[MKV] Cannot find tracks\n");
    return 0;
  }
  /* And analyze them */
  if(!analyzeTracks(&ebml,alen))
  {
      printf("[MKV] incorrect tracks\n");
  }
  printf("[MKV] Tracks analyzed\n");
  if(!_isvideopresent)
  {
    printf("[MKV] No video\n");
    return 0;
  }
  printf("[MKV] Indexing clusters\n");
  if(!indexClusters(&ebml))
  {
    printf("[MKV] Cluster indexing failed\n");
    return 0;
  }
  printf("[MKV]Found %u clusters\n",_nbClusters);
  printf("[MKV] Indexing video\n");
    if(!videoIndexer(&ebml))
    {
      printf("[MKV] Video indexing failed\n");
      return 0;
    }
  // update some infos
  _videostream.dwLength= _mainaviheader.dwTotalFrames=_tracks[0].index.size();;

  _parser=new ADM_ebml_file();
  ADM_assert(_parser->open(name));
  _filename=ADM_strdup(name);

  // Finaly update index with queue
  float duration=_videostream.dwLength*_tracks[0]._defaultFrameDuration;
  duration/=1000;
  uint32_t duration32=(uint32_t)duration;
  printf("[MKV] Video Track duration %u ms\n",_videostream.dwLength);
  // Useless.....readCue(&ebml);
  for(int i=0;i<_nbAudioTrack;i++)
  {
    rescaleTrack(&(_tracks[1+i]),duration32);
    if(_tracks[1+i].wavHeader.encoding==WAV_OGG_VORBIS)
    {
        printf("[MKV] Reformatting vorbis header for track %u\n",i);
        reformatVorbisHeader(&(_tracks[1+i]));
    }
  }
    _access=new mkvAccess *[_nbAudioTrack];
    _audioStreams=new ADM_audioStream *[_nbAudioTrack];
    for(int i=0;i<_nbAudioTrack;i++)
    {
        _access[i]=new mkvAccess(_filename,&(_tracks[i+1]));
        _audioStreams[i]=ADM_audioCreateStream(&(_tracks[1+i].wavHeader), _access[i]);;
    }

  printf("[MKV]Matroska successfully read\n");

  return 1;
}
/**
    \fn rescaleTrack
    \brief Compute the average duration of one audio frame if the info is not present in the stream

*/
uint8_t mkvHeader::rescaleTrack(mkvTrak *track,uint32_t durationMs)
{
        if(track->_defaultFrameDuration) return 1; // No need to change
        float samples=1000.;
        samples*=durationMs;
        samples/=track->nbFrames;  // 1000 * sample per packet
        track->_defaultFrameDuration=(uint32_t)samples;
        return 1;

}
/**
    \fn checkHeader
    \brief Check that we are compatible with that version of matroska. At the moment, just dump some infos.
*/
uint8_t mkvHeader::checkHeader(void *head,uint32_t headlen)
{
  printf("[MKV] *** Header dump ***\n");
 ADM_ebml_file father( (ADM_ebml_file *)head,headlen);
 walk(&father);
 printf("[MKV] *** End of Header dump ***\n");
 return 1;

}
/**
    \fn analyzeTracks
    \brief Read Tracks Info.
*/
uint8_t mkvHeader::analyzeTracks(void *head,uint32_t headlen)
{
  uint64_t id,len;
  ADM_MKV_TYPE type;
  const char *ss;
 ADM_ebml_file father( (ADM_ebml_file *)head,headlen);
 while(!father.finished())
 {
      father.readElemId(&id,&len);
      if(!ADM_searchMkvTag( (MKV_ELEM_ID)id,&ss,&type))
      {
        printf("[MKV] Tag 0x%"LLX" not found (len %"LLU")\n",id,len);
        father.skip(len);
        continue;
      }
      ADM_assert(ss);
      if(id!=MKV_TRACK_ENTRY)
      {
        printf("[MKV] skipping %s\n",ss);
        father.skip(len);
        continue;
      }
      if(!analyzeOneTrack(&father,len)) return 0;
 }
 return 1;
}

/**
    \fn walk
    \brief Walk a matroska atom and print out what is found.
*/
uint8_t mkvHeader::walk(void *seed)
{
  uint64_t id,len;
  ADM_MKV_TYPE type;
  const char *ss;

   ADM_ebml_file *father=(ADM_ebml_file *)seed;
    while(!father->finished())
   {
      father->readElemId(&id,&len);
      if(!ADM_searchMkvTag( (MKV_ELEM_ID)id,&ss,&type))
      {
        printf("[MKV] Tag 0x%"LLX" not found (len %"LLU")\n",id,len);
        father->skip(len);
        continue;
      }
      ADM_assert(ss);
      switch(type)
      {
        case ADM_MKV_TYPE_CONTAINER:
                  father->skip(len);
                  printf("%s skipped\n",ss);
                  break;
        case ADM_MKV_TYPE_UINTEGER:
                  printf("%s:%"LLU"\n",ss,father->readUnsignedInt(len));
                  break;
        case ADM_MKV_TYPE_INTEGER:
                  printf("%s:%"LLD"\n",ss,father->readSignedInt(len));
                  break;
        case ADM_MKV_TYPE_STRING:
        {
                  char string[len+1];
                  string[0]=0;
                  father->readString(string,len);
                  printf("%s:<%s>\n",ss,string);
                  break;
        }
        default:
                printf("%s skipped\n",ss);
                father->skip(len);
                break;
      }
   }
  return 1;
}



/**
   \fn Dump
*/

void mkvHeader::Dump(void)
{

}

/**
    \fn close
*/

uint8_t mkvHeader::close(void)
{
  if(_clusters)
  {
    delete [] _clusters;
    _clusters=NULL;
  }
  // CLEANUP!!
  if(_parser) delete _parser;
  _parser=NULL;


#define FREEIF(i) { if(_tracks[i].extraData) delete [] _tracks[i].extraData; _tracks[i].extraData=0;}
  if(_isvideopresent)
  {
      FREEIF(0);
  }
  for(int i=0;i<_nbAudioTrack;i++)
  {
    FREEIF(1+i);
  }

    if(_audioStreams)
    {
        for(int i=0;i<_nbAudioTrack;i++) if(_audioStreams[i]) delete _audioStreams[i];
        delete [] _audioStreams;
        _audioStreams=NULL;
    }
    if(_access)
    {
        for(int i=0;i<_nbAudioTrack;i++) if(_access[i]) delete _access[i];
        delete [] _access;
        _access=NULL;
    }

}
/**
    \fn mkvHeader
    \brief constructor
*/

 mkvHeader::mkvHeader( void ) : vidHeader()
{
  _parser=NULL;
  _nbAudioTrack=0;
  _filename=NULL;
  memset(_tracks,0,sizeof(_tracks));
  _reordered=0;

  _clusters=NULL;
  _clustersCeil=0;
  _nbClusters=0;
  _currentAudioTrack=0;
  _access=NULL;
  _audioStreams=NULL;
}
/**
    \fn ~mkvHeader
    \brief constructor
*/
 mkvHeader::~mkvHeader(  )
{
  close();
}

/**
    \fn setFlag
    \brief setFlag
*/

  uint8_t  mkvHeader::setFlag(uint32_t frame,uint32_t flags)
{
  if(frame>=_tracks[0].index.size()) return 0;
  _tracks[0].index[frame].flags=flags;
  return 1;
}

/**
    \fn getFlags
    \brief getFlags
*/
uint32_t mkvHeader::getFlags(uint32_t frame,uint32_t *flags)
{
  if(frame>=_tracks[0].index.size()) return 0;
  *flags=_tracks[0].index[frame].flags;
  if(!frame) *flags=AVI_KEY_FRAME;
  return 1;
}
/**
    \fn getTime
*/
uint64_t mkvHeader::getTime(uint32_t frame)
{
 if(frame>=_tracks[0].index.size()) return ADM_COMPRESSED_NO_PTS;
  return _tracks[0].index[frame].Pts;
}
/**
    \fn getVideoDuration

*/
uint64_t mkvHeader::getVideoDuration(void)
{
    uint32_t limit=_tracks[0].index.size();
    if(!limit) return 0;
    return _tracks[0].index[limit-1].Pts;
}

/**
    \fn getFrameSize
*/
uint8_t                 mkvHeader::getFrameSize(uint32_t frame,uint32_t *size)
{
    if(frame>=_tracks[0].index.size()) return 0;
    *size=_tracks[0].index[frame].size;
    return 1;
}

/**
  \fn getFrameNoAlloc
   \brief Read a video frames, return size & flags
*/

uint8_t  mkvHeader::getFrame(uint32_t framenum,ADMCompressedImage *img)
{
  ADM_assert(_parser);
  if(framenum>=_tracks[0].index.size()) return 0;

  mkvIndex *dx=&(_tracks[0].index[framenum]);

  _parser->seek(dx->pos);
  _parser->readSignedInt(2); // Timecode
  _parser->readu8();  // flags

  _parser->readBin(img->data,dx->size-3);
  img->dataLength=dx->size-3;

  img->flags=dx->flags;
  img->demuxerDts=dx->Dts;
  img->demuxerPts=dx->Pts;
  if(!framenum) img->flags=AVI_KEY_FRAME;


  return 1;
}
/**
    \fn getExtraHeaderData
    \brief Returns extra data infos
*/
uint8_t  mkvHeader::getExtraHeaderData(uint32_t *len, uint8_t **data)
{
                *len=_tracks[0].extraDataLen;
                *data=_tracks[0].extraData;
                return 1;
}

/**
    \fn mkreformatVorbisHeader
    \brief reformat oggvorbis header to avidemux style
*/
uint8_t mkvHeader::reformatVorbisHeader(mkvTrak *trk)
{
  /*
  The private data contains the first three Vorbis packet in order. The lengths of the packets precedes them. The actual layout is:
Byte 1: number of distinct packets '#p' minus one inside the CodecPrivate block. This should be '2' for current Vorbis headers.
Bytes 2..n: lengths of the first '#p' packets, coded in Xiph-style lacing. The length of the last packet is the length of the CodecPrivate block minus the lengths coded in these bytes minus one.
Bytes n+1..: The Vorbis identification header, followed by the Vorbis comment header followed by the codec setup header.
  */
  uint8_t *oldata=trk->extraData;
  uint32_t oldlen=trk->extraDataLen;
  uint32_t len1,len2,len3;
  uint8_t *head;
      if(*oldata!=2) {printf("[MKV] weird audio, expect problems\n");return 0;}
      // First packet length
      head=oldata+1;
#define READ_LEN(x) \
      x=0; \
      while(*head==0xff)  \
      { \
        x+=0xff; \
        head++; \
      } \
      x+=*head++;

      READ_LEN(len1);
      READ_LEN(len2);
      len3=oldata+oldlen-head;
      if(len3<=len1+len2)
      {
        printf("Error in vorbis header, len3 too small %u %u / %u\n",len1,len2,len3);
        return 0;
      }
      len3-=(len1+len2);
      printf("Found packet len : %u %u %u, total size %u\n",len1,len2,len3,oldlen);
      // Now build our own packet...
      uint8_t *nwdata=new uint8_t[len1+len2+len3+sizeof(uint32_t)*3];
      uint32_t nwlen=len1+len2+len3+sizeof(uint32_t)*3;
      uint8_t *cp=nwdata+sizeof(uint32_t)*3;
      memcpy(cp,head,len1);
      memcpy(cp+len1,head+len1,len2);
      memcpy(cp+len1+len2,head+len1+len2,len3);

      uint32_t *h=(uint32_t *)nwdata;
      h[0]=len1;
      h[1]=len2;
      h[2]=len3;
      // Destroy old datas
      delete [] oldata;
      trk->extraData=nwdata;
      trk->extraDataLen=nwlen;
  return 1;
}
/**
    \fn getNbAudioStreams
*/
uint8_t                 mkvHeader::getNbAudioStreams(void)
{
    return _nbAudioTrack;
}
/**
    \fn getAudioInfo
*/

WAVHeader              *mkvHeader::getAudioInfo(uint32_t i )
{
    if(!_nbAudioTrack) return NULL;
    ADM_assert(i<_nbAudioTrack)
    return &(_tracks[1+i].wavHeader);
}
/**
    \fn getAudioStream
*/
uint8_t                 mkvHeader::getAudioStream(uint32_t i,ADM_audioStream  **audio)
{
 if(_nbAudioTrack)
  {
      ADM_assert(i<_nbAudioTrack)
      *audio=_audioStreams[i];
      return 1;
  }
  *audio=NULL;
  return 0;

}


/**
    \fn getPtsDts
*/
bool    mkvHeader::getPtsDts(uint32_t frame,uint64_t *pts,uint64_t *dts)
{
     ADM_assert(_parser);
     if(frame>=_tracks[0].index.size()) 
     {
            printf("[MKV] Frame %"LU" exceeds # of frames %"LU"\n",frame,(uint32_t)_tracks[0].index.size());
            return false;
     }
    mkvIndex *dx=&(_tracks[0].index[frame]);
    
    *dts=dx->Dts; // FIXME
    *pts=dx->Pts;
    return true;
}
/**
        \fn setPtsDts
*/
bool    mkvHeader::setPtsDts(uint32_t frame,uint64_t pts,uint64_t dts)
{
      ADM_assert(_parser);
     if(frame>=_tracks[0].index.size()) 
     {
            printf("[MKV] Frame %"LU" exceeds # of frames %"LU"\n",frame,(uint32_t)_tracks[0].index.size());
            return false;
     }
    mkvIndex *dx=&(_tracks[0].index[frame]);
    
    dx->Dts=dts; // FIXME
    dx->Pts=pts;
    

}



//****************************************
//EOF
