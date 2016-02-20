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
#include "ADM_cpp.h"
#include <math.h>

#include "ADM_default.h"
#include "ADM_Video.h"
#include "ADM_mkv.h"
#include "ADM_codecType.h"
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

  _timeBase=1000; // default value is 1 ms timebase (unit is in us)
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
/* Read info to get the timeScale if it exists... */
  if(ebml.find(ADM_MKV_SECONDARY,MKV_SEGMENT,MKV_INFO,&alen))
  {
        ADM_ebml_file father( &ebml,alen);
        uint64_t timeBase=walkAndFind(&father,MKV_TIMECODE_SCALE);
        if(timeBase)
        {
            ADM_info("TimeBase found : %"PRIu64" ns\n",timeBase);
            _timeBase=timeBase/1000; // We work in us
        }
  }
  /* --*/
  if(!ebml.simplefind(MKV_SEGMENT,&len,true))
  {
      printf("[MKV] Cannot find Segment\n");      
      return false;
  }
  _segmentPosition=ebml.tell();
  printf("[MKV] found Segment at 0x%llx\n",(uint64_t)_segmentPosition);
   /* Now find tracks */
  if(ebml.find(ADM_MKV_SECONDARY,MKV_SEGMENT,MKV_SEEK_HEAD,&alen))
  {
       ADM_ebml_file seekHead( &ebml,alen);
       readSeekHead(&seekHead);       
  }
  /* Now find tracks */
  /* And analyze them */
  if(!analyzeTracks(&ebml))
  {
      printf("[MKV] incorrect tracks\n");
  }
  printf("[MKV] Tracks analyzed\n");
  if(!_isvideopresent)
  {
    printf("[MKV] No video\n");
    return 0;
  }
  readCue(&ebml);
  printf("[MKV] Indexing clusters\n");
  if(!indexClusters(&ebml))
  {
    printf("[MKV] Cluster indexing failed\n");
    return 0;
  }  
  printf("[MKV]Found %u clusters\n",_clusters.size());
  printf("[MKV] Indexing video\n");
    if(!videoIndexer(&ebml))
    {
      printf("[MKV] Video indexing failed\n");
      return 0;
    }
  // update some infos
  _videostream.dwLength= _mainaviheader.dwTotalFrames=_tracks[0].index.size();;

    if(!isH264Compatible(_videostream.fccHandler) && !isMpeg4Compatible(_videostream.fccHandler) && !isMpeg12Compatible(_videostream.fccHandler))
    {
        updateFlagsWithCue();
    }
    _cueTime.clear();
  
  
  _parser=new ADM_ebml_file();
  ADM_assert(_parser->open(name));
  _filename=ADM_strdup(name);

  // Now dump some infos about the track
    for(int i=0;i<1+_nbAudioTrack;i++)
        ADM_info("Track %"PRIu32" has an index size of %d entries\n",i,_tracks[i].index.size());


  // Delay frames + recompute frame duration
// now that we have a good frameduration and max pts dts difference, we can set a proper DTS for all video frame
    uint32_t ptsdtsdelta, mindelta;
    bool hasBframe;
  ComputeDeltaAndCheckBFrames(&mindelta, &ptsdtsdelta,&hasBframe);
  
  
  int last=_tracks[0].index.size();
  uint64_t increment=_tracks[0]._defaultFrameDuration;
  uint64_t lastDts=0;
  _tracks[0].index[0].Dts=0;
  mkvTrak                 *vid=_tracks;
  if(hasBframe==true) // Try to compute a sane DTS knowing the PTS and the DTS/PTS delay
    {
      for(int i=1;i<last;i++)
      {
            uint64_t pts,dts;
            pts=vid->index[i].Pts;
            lastDts+=increment; // This frame dts with no correction
            if(pts==ADM_NO_PTS)
            {
                vid->index[i].Dts=lastDts;
                continue;
            }
            uint64_t limitDts=vid->index[i].Pts-ptsdtsdelta;
            if(  lastDts<limitDts)
            {
                lastDts=limitDts;
            }
            vid->index[i].Dts=lastDts;
      }
        // Check that we have PTS>=DTS also
        uint64_t enforePtsGreaterThanDts=0;
        for(int i=0;i<last;i++)
        {
                if(vid->index[i].Pts<vid->index[i].Dts)
                {
                    uint64_t delta=vid->index[i].Dts-vid->index[i].Pts;
                    if(delta>enforePtsGreaterThanDts) enforePtsGreaterThanDts=delta;
                }
        }
        if(enforePtsGreaterThanDts)
        {
                ADM_info("Have to delay by %"PRIu32" ms so that PTS>DTS\n",enforePtsGreaterThanDts);
                for(int i=0;i<_nbAudioTrack+1;i++)
                delayTrack(i,&(_tracks[i]),enforePtsGreaterThanDts);
        }
    }else
    {       // No bframe, DTS=PTS
      for(int i=0;i<last;i++)
      {
            vid->index[i].Dts=vid->index[i].Pts;
      }
    }


  if(last)
  {
          float duration=_tracks[0].index[last-1].Pts;
          duration/=1000;
          uint32_t duration32=(uint32_t)duration;
          printf("[MKV] Video Track duration for %u ms\n",duration32);
          // Useless.....

        

          for(int i=0;i<_nbAudioTrack;i++)
          {
            rescaleTrack(&(_tracks[1+i]),duration32);
            if(_tracks[1+i].wavHeader.encoding==WAV_OGG_VORBIS)
            {
                printf("[MKV] Reformatting vorbis header for track %u\n",i);
                reformatVorbisHeader(&(_tracks[1+i]));
            }
          }
    }
    _access=new mkvAccess *[_nbAudioTrack];
    _audioStreams=new ADM_audioStream *[_nbAudioTrack];
    for(int i=0;i<_nbAudioTrack;i++)
    {
        _access[i]=new mkvAccess(_filename,&(_tracks[i+1]));
        _audioStreams[i]=ADM_audioCreateStream(&(_tracks[1+i].wavHeader), _access[i]);;
        _audioStreams[i]->setLanguage(_tracks[1+i].language);
    }
  //dumpVideoIndex(200);
  printf("[MKV]Matroska successfully read\n");
  return 1;
}
/**
    \fn delayFrameIfBFrames
    \brief delay audio and video by 2 * time increment if b frames present
                else we may have PTS<DTS
*/
bool mkvHeader::delayTrack(int index,mkvTrak *track, uint64_t value)
{
    int nb=track->index.size();
    for(int i=0;i<nb;i++)
    {
        if(track->index[i].Pts!=ADM_NO_PTS) track->index[i].Pts+=value;
        if(index) // Must also delay DTS for audio as we use DTS not PTS
            if(track->index[i].Dts!=ADM_NO_PTS) track->index[i].Dts+=value;
    }
    return true;
}
/**
    \fn delayFrameIfBFrames
    \brief recompute max pts/dts distance and delay all tracks if needed
    we dont want a negative dts.
    \return maxdelta in us
*/
bool mkvHeader::ComputeDeltaAndCheckBFrames(uint32_t *minDeltaX, uint32_t *maxDeltaX, bool *bFramePresent)
{
    mkvTrak *track=_tracks;
    int nb=track->index.size();
    int nbBFrame=0;
    int64_t delta,maxDelta=0;
    int64_t minDelta=100000000;
    *bFramePresent=false;
    if(nb>1)
    {
        bool monotone=true;
        uint64_t pts=track->index[0].Pts;
        for(int i=1;i<nb;i++)
        {
            if(track->index[i].Pts<pts) 
            {
                monotone=false;
                break;
            }
            pts=track->index[i].Pts;
        }
        if(monotone==true)
        {
            ADM_info("PTS is monotonous, probably no bframe\n");        
            *bFramePresent=false;
        }else
        {
            ADM_info("PTS is not monotonous, there are bframe\n");
            *bFramePresent=true;
        }
    }

    if(nb>1)
    {
        // Search minimum and maximum between 2 frames
        // the minimum will give us the maximum fps
        // the maximum will give us the max PTS-DTS delta so that we can compute DTS
        for(int i=0;i<nb-1;i++) 
        {
            if(track->index[i].flags==AVI_B_FRAME) nbBFrame++;
            delta=(int64_t)track->index[i+1].Pts-(int64_t)track->index[i].Pts;
            if(delta<0) delta=-delta;
            if(delta<minDelta) minDelta=delta;
            if(delta>maxDelta) maxDelta=delta;
            //printf("\/=%"PRId64" Min %"PRId64" MAX %"PRId64"\n",delta,minDelta,maxDelta);
        }
    }
    if(nbBFrame) *bFramePresent=true;
    ADM_info("Minimum delta found %"PRId64" us\n",minDelta);
    ADM_info("Maximum delta found %"PRId64" us\n",maxDelta);
    if(minDelta)
    {
        if(minDelta<track->_defaultFrameDuration && labs((long int)minDelta-(long int)track->_defaultFrameDuration)>1000)
        {
            ADM_info("Changing default frame duration from %"PRIu64" to %"PRIu64" us\n",
                    track->_defaultFrameDuration,minDelta);
            track->_defaultFrameDuration=minDelta;
            // updated fps also
            float f=minDelta;
            f=1000000./f;
            _videostream.dwScale=1000;
            _videostream.dwRate=(uint32_t)(f*1000.);
        }else
        {
            ADM_info("Keeping default frame duration  %"PRIu64" us\n", track->_defaultFrameDuration);
        }

    }
    ADM_info("First frame pts     %"PRId64" us\n",track->index[0].Pts);
    uint64_t adj=0;
    int limit=32;
    if(limit>nb) limit=nb;
    // Pts must be >= maxDelta for all frames, the first 32 will do
    for(int i=0;i<limit;i++)
    {
        if(maxDelta>track->index[i].Pts) 
        {
            uint64_t newAdj=maxDelta-track->index[i].Pts;
            if(newAdj>adj) adj=newAdj;
        }
    }
    if(adj) // need to correct
    {
        ADM_info("Delaying video by %"PRIu64" us\n",adj);
        ADM_info("[mkv] Delaying audio by %"PRIu64" us\n",adj);
        for(int i=0;i<_nbAudioTrack+1;i++)
            delayTrack(i,&(_tracks[i]),adj);
    }
    *maxDeltaX=maxDelta;
    *minDeltaX=minDelta;
    return true;
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
 * \fn goBeforeAtomAtPosition
 * \brief check and position the read at the payload for atom searchedId, return payloadSize in outputLen
 */
bool mkvHeader::goBeforeAtomAtPosition(ADM_ebml_file *parser, uint64_t position,uint64_t &outputLen, MKV_ELEM_ID searchedId,const char *txt)
{
    uint64_t id,len;
    ADM_MKV_TYPE type;
    const char *ss;

    if(!position)
    {
        ADM_warning("No offset available for %s\n",txt);
        return false;
    }
    parser->seek(position);
    if(!parser->readElemId(&id,&len))
    {
        ADM_warning("No element  available for %s\n",txt);
        return false;
    }
    if(!ADM_searchMkvTag( (MKV_ELEM_ID)id,&ss,&type))
    {
      printf("[MKV/SeekHead] Tag 0x%"PRIx64" not found (len %"PRIu64")\n",id,len);
      return false;
    }
    if(id!=searchedId)
    {
        printf("Found %s instead of %s, ignored \n",ss,txt);
        return false;
    }    
    outputLen=len;
    return true;
}

/**
    \fn analyzeTracks
    \brief Read Tracks Info.
*/
bool mkvHeader::analyzeTracks(ADM_ebml_file *parser)
{
  uint64_t len;
  uint64_t id;
  const char *ss;
  ADM_MKV_TYPE type;
  
  if(!goBeforeAtomAtPosition(parser, _trackPosition,len, MKV_TRACKS,"MKV_TRACKS"))
  {
      ADM_warning("Cannot go to the TRACKS atom\n");
      return false;
  }
  
    ADM_ebml_file father( parser,len);
    while(!father.finished())
    {
      father.readElemId(&id,&len);
      if(!ADM_searchMkvTag( (MKV_ELEM_ID)id,&ss,&type))
      {
        printf("[MKV] Tag 0x%"PRIx64" not found (len %"PRIu64")\n",id,len);
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
        printf("[MKV] Tag 0x%"PRIx64" not found (len %"PRIu64")\n",id,len);
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
                  printf("%s:%"PRIu64"\n",ss,father->readUnsignedInt(len));
                  break;
        case ADM_MKV_TYPE_INTEGER:
                  printf("%s:%"PRId64"\n",ss,father->readSignedInt(len));
                  break;
        case ADM_MKV_TYPE_STRING:
        {
                  char *string=new char[len+1];
                  string[0]=0;
                  father->readString(string,len);
                  printf("%s:<%s>\n",ss,string);
                  delete [] string;
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
    \fn walk
    \brief Walk a matroska atom and print out what is found.
*/
uint64_t mkvHeader::walkAndFind(void *seed,MKV_ELEM_ID searched)
{
  uint64_t id,len;
  ADM_MKV_TYPE type;
  const char *ss;
   ADM_ebml_file *father=(ADM_ebml_file *)seed;
  uint64_t value=0;
    while(!father->finished())
   {
      father->readElemId(&id,&len);
      if(!ADM_searchMkvTag( (MKV_ELEM_ID)id,&ss,&type))
      {
        printf("[MKV] Tag 0x%"PRIx64" not found (len %"PRIu64")\n",id,len);
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
                    {
                  uint64_t v=father->readUnsignedInt(len);
                  if(id==searched)
                        value=v;
                  printf("%s:%"PRIu64"\n",ss,v);
                  }
                  break;
        case ADM_MKV_TYPE_INTEGER:
                  printf("%s:%"PRId64"\n",ss,father->readSignedInt(len));
                  break;
        case ADM_MKV_TYPE_STRING:
        {
                  char *string=new char[len+1];
                  string[0]=0;
                  father->readString(string,len);
                  printf("%s:<%s>\n",ss,string);
                  delete [] string;
                  break;
        }
        default:
                printf("%s skipped\n",ss);
                father->skip(len);
                break;
      }
   }
  return value;
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
    _clusters.clear();
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
    return 1;
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
 // memset(_tracks,0,sizeof(_tracks));
  _reordered=0;

  _currentAudioTrack=0;
  _access=NULL;
  _audioStreams=NULL;
  
  readBuffer=NULL;
  _cuePosition=0;
  _segmentPosition=0;
  _trackPosition=0;
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
    return _tracks[0].index[limit-1].Pts+_tracks[0]._defaultFrameDuration;
}

/**
    \fn getFrameSize
*/
uint8_t                 mkvHeader::getFrameSize(uint32_t frame,uint32_t *size)
{
    if(frame>=_tracks[0].index.size()) return 0;
    *size=_tracks[0].index[frame].size+_tracks[0].headerRepeatSize;
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
  img->dataLength=readAndRepeat(0,img->data, dx->size-3);
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
            printf("[MKV] Frame %"PRIu32" exceeds # of frames %"PRIu32"\n",frame,(uint32_t)_tracks[0].index.size());
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
            printf("[MKV] Frame %"PRIu32" exceeds # of frames %"PRIu32"\n",frame,(uint32_t)_tracks[0].index.size());
            return false;
     }
    mkvIndex *dx=&(_tracks[0].index[frame]);
    
    dx->Dts=dts; // FIXME
    dx->Pts=pts;
    return true;

}
/**
 * \fn readSeekHead
 * \bried used to locate the interesting parts of the file
 */
bool    mkvHeader::readSeekHead(ADM_ebml_file *body)
{
    uint64_t vlen,len;
    ADM_info("Parsing SeekHead\n");
    while(!body->finished())
    {
        if(!body->simplefind(MKV_SEEK,&vlen,false))
             break;         
        ADM_ebml_file item(body,vlen);              
        uint64_t id;
        ADM_MKV_TYPE type;
        const char *ss;

        item.readElemId(&id,&len);
        if(!ADM_searchMkvTag( (MKV_ELEM_ID)id,&ss,&type))
        {
          printf("[MKV/SeekHead] Tag 0x%"PRIx64" not found (len %"PRIu64")\n",id,len);
          return false;
        }
        if(id!=MKV_ID)
        {
          printf("Found %s in CUES, ignored \n",ss);
          item.skip(len);
          return false;
        }
        // read id
        uint64_t t=item.readEBMCode_Full();
        if(!ADM_searchMkvTag( (MKV_ELEM_ID)t,&ss,&type))
        {
          printf("[MKV/SeekHead] Tag 0x%"PRIx64" not found (len %"PRIu64")\n",id,len);
          return false;
        }
        ADM_info("Found entry for %s\n",ss);
        item.readElemId(&id,&len);
        if(!ADM_searchMkvTag( (MKV_ELEM_ID)id,&ss,&type))
        {
          printf("[MKV/SeekHead] Tag 0x%"PRIx64" not found (len %"PRIu64")\n",id,len);
          return false;
        }
        if(id!=MKV_SEEK_POSITION)
        {
          printf("Found %s in CUES, ignored \n",ss);
          item.skip(len);
          return false;
        }
        uint64_t position=item.readUnsignedInt(len);
        switch(t)
        {
            case MKV_CUES:
                    _cuePosition=position+_segmentPosition;
                    ADM_info("   at position  0x%llx\n",_cuePosition);
                    break;
            case MKV_TRACKS:
                    _trackPosition=position+_segmentPosition;;
                    ADM_info("   at position at 0x%llx\n",_trackPosition);
            case MKV_INFO:
            default:
                    break;
        }
              
    }
    ADM_info("Parsing SeekHead done successfully\n");
    if(!_trackPosition )
        return false;
    return true;
}

//****************************************
//EOF
