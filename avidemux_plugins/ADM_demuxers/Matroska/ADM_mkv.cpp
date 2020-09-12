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
#include <stdlib.h>
#include "ADM_default.h"
#include "ADM_Video.h"
#include "ADM_mkv.h"
#include "ADM_codecType.h"
#include "mkv_tags.h"
#include "ADM_audioXiphUtils.h"
#include "ADM_vidMisc.h"
#include "ADM_mkvDeviation.h"
typedef struct
{
    int durationInUs;
    int num;
    int den;
}frameRateStruct;

static const frameRateStruct candidateFrameRate[]=
{
   {41708,1001,24000}, // 23.976 fps
   //{41667,1000,24000}, // 24 fps
   {40000,1000,25000}, // 25 fps
   {33367,1001,30000}, // 30 NTSC
   //{20853,2000,24000}, // 24*2
   {20854,1001,48000}, // 23976*2
   {20000,1000,50000}, // 50
   {16683,1001,60000}, // 60 NTSCs
   {16667,1000,60000} // 60 fps
};

static int getStdFrameRate(int interval)
{
  int n=sizeof(candidateFrameRate)/sizeof(frameRateStruct);
  int delta=1000; // 1000 us error allowed
  int bestMatch=-1;
  for(int i=0;i<n;i++)
  {
      int er=abs(interval-candidateFrameRate[i].durationInUs);
      if(er<1000) // 10 us error
      {
        if(er<delta)
        {
          delta=er;
          bestMatch=i;
        }
      }
  }
  ADM_info("Best match is %d\n",bestMatch);
  return bestMatch;
}


/**
    \fn open
    \brief Try to open the mkv file given as parameter

*/

uint8_t mkvHeader::open(const char *name)
{

  ADM_ebml_file ebml;
  uint64_t len;
  uint64_t alen;

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
            ADM_info("TimeBase found : %" PRIu64" ns\n",timeBase);
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
  ADM_info("[MKV] found Segment at 0x%llx\n",(uint64_t)_segmentPosition);
   /* Now find tracks */
  bool tracksElemFound=false;
  uint64_t nextHead=0;
  if(ebml.find(ADM_MKV_SECONDARY,MKV_SEGMENT,MKV_SEEK_HEAD,&alen))
  {
       ADM_ebml_file seekHead( &ebml,alen);
       tracksElemFound = readSeekHead(&seekHead,&nextHead);
       if(!tracksElemFound && nextHead)
       {
           ebml.seek(nextHead);
           if(ebml.simplefind(MKV_SEEK_HEAD,&len,false))
           {
               ADM_ebml_file secondary(&ebml,len);
               tracksElemFound = readSeekHead(&secondary);
           }
       }
  }
  if(!tracksElemFound)
  { // No valid seek_head, try to find MKV_TRACKS
      ADM_info("Searching for MKV_TRACKS\n");
      int headerLen;
      uint64_t bodyLen;
      if(!ebml.findContainerOfSecondary(MKV_SEGMENT,MKV_TRACKS,true, &_trackPosition, &headerLen,&bodyLen))
      {
          ADM_warning("Cannot locate MKV_TRACKS\n");
          return false;
      }
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
  uint8_t result=indexClusters(&ebml);
  if(result!=1)
  {
    if(!result)
        printf("[MKV] Cluster indexing failed\n");
    return result;
  }
  printf("[MKV]Found %u clusters\n",_clusters.size());
  printf("[MKV] Indexing video\n");
    result=videoIndexer(&ebml);
    if(result!=1)
    {
        if(!result)
            printf("[MKV] Video indexing failed\n");
        return result;
    }
  // update some infos
  _videostream.dwLength= _mainaviheader.dwTotalFrames=_tracks[0].index.size();;
    if(! _videostream.dwLength)
    {
        ADM_warning("No image found in this video\n");
        return 0;
    }

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
        ADM_info("Track %" PRIu32" has an index size of %d entries\n",i,_tracks[i].index.size());

    int last=_tracks[0].index.size();
#if 0
    if(isVC1Compatible(_videostream.fccHandler))
    {
        mkvTrak *vid=_tracks;

        ADM_warning("Deleting timestamps. For VC1, they are often wrong\n");
        for(int i=1;i<last-1;i++)
            vid->index[i].Pts=ADM_NO_PTS;
    }
#endif
  // Delay frames + recompute frame duration
// now that we have a good frameduration and max pts dts difference, we can set a proper DTS for all video frame
    uint32_t ptsdtsdelta, mindelta;
    bool hasBframe=false;
    if(last>1)
        ComputeDeltaAndCheckBFrames(&mindelta, &ptsdtsdelta, &hasBframe);

    uint64_t increment=_tracks[0]._defaultFrameDuration;
    uint64_t lastDts=0;
    _tracks[0].index[0].Dts=0;
    mkvTrak *vid=_tracks;
    if(hasBframe==true) // Try to compute a sane DTS knowing the PTS and the DTS/PTS delay
    {
        setDtsFromListOfSortedPts();

        uint64_t enforcePtsGreaterThanDts=0;
        int frmeMaxDlta=-1;
        for(int i=0;i<last;i++)
        {
            if(vid->index[i].Pts<vid->index[i].Dts)
            {
                uint64_t delta=vid->index[i].Dts-vid->index[i].Pts;
                if(delta>enforcePtsGreaterThanDts)
                {
                    enforcePtsGreaterThanDts=delta;
                    frmeMaxDlta=i;
                }
            }
        }
        if(enforcePtsGreaterThanDts)
        {
            ADM_info("Have to delay by %" PRIu64" us detected for frame %d so that PTS>DTS\n",enforcePtsGreaterThanDts,frmeMaxDlta);
            for(int i=0;i<_nbAudioTrack+1;i++)
                delayTrack(i,&(_tracks[i]),enforcePtsGreaterThanDts);
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
      for(int i=last-32;i<last;i++)
      {
          if(i<0) continue;
          if(_tracks[0].index[i].Pts==ADM_NO_PTS) continue;
          if(_tracks[0].duration<_tracks[0].index[i].Pts)
              _tracks[0].duration=_tracks[0].index[i].Pts;
      }
      _tracks[0].duration+=increment;
      uint32_t duration32=(uint32_t)((_tracks[0].duration+500)/1000);
      printf("[MKV] Video track duration: %s (%u ms)\n",ADM_us2plain(_tracks[0].duration),duration32);
      // Useless.....

          for(int i=0;i<_nbAudioTrack;i++)
          {
            rescaleTrack(&(_tracks[1+i]),duration32);
            if(!_tracks[1+i].duration)
                _tracks[1+i].duration=_tracks[0].duration; // FIXME
            switch(_tracks[1+i].wavHeader.encoding)
            {
                case WAV_OGG_VORBIS:            
                    ADM_info("[MKV] Reformatting vorbis header for track %u\n",i);
                    reformatVorbisHeader(&(_tracks[1+i]));
                    break;
                case WAV_PCM:
                case WAV_LPCM:
                    ADM_info("[MKV] Checking PCM block size for track %i\n",i);
                    _tracks[i+1]._needBuffering=isBufferingNeeded(&(_tracks[i+1]));
                    break;
                default:
                    break;
                
            }
            
            
          }
    }
    _access=new ADM_audioAccess *[_nbAudioTrack];
    _audioStreams=new ADM_audioStream *[_nbAudioTrack];
    for(int i=0;i<_nbAudioTrack;i++)
    {
        if(_tracks[i+1]._needBuffering)
        {
            mkvAccess *access=new mkvAccess(_filename,&(_tracks[i+1]));
            _access[i]=new mkvAccessBuffered(access,_tracks[i+1]._needBuffering);
        }else if(_tracks[i+1].wavHeader.encoding==MKV_MUX_LATM)
        {
            mkvAccess *access=new mkvAccess(_filename,&(_tracks[i+1]));
            _access[i]=new mkvAccessLatm(access,LATM_MAX_BUFFER_SIZE);
            _tracks[i+1].wavHeader.encoding=WAV_AAC;
        }else
        {
            _access[i]=new mkvAccess(_filename,&(_tracks[i+1]));
        }
        _audioStreams[i]=ADM_audioCreateStream(&(_tracks[1+i].wavHeader), _access[i]);;
        _audioStreams[i]->setLanguage(_tracks[1+i].language);
    }
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
    \fn setDtsFromListOfSortedPts
    \brief Handle cases of mixed content and wrong default time increment
*/
bool mkvHeader::setDtsFromListOfSortedPts(void)
{
    mkvTrak *vid=_tracks;
    int last=_tracks[0].index.size();
    int nbValidPts=_sortedPts.size();
    int nbPtsUnset=_framesNoPts.size();
    if(!nbPtsUnset)
    {
        ADM_assert(last==nbValidPts);
        for(int i=0;i<last;i++)
            vid->index[i].Dts=_sortedPts.at(i);
    }else
    {
        int start=0;
        while(start<nbPtsUnset && _framesNoPts.at(start)==start) start++;
        // The first entry in the sorted list is frame "start", how many consecutive frames with pts do we have?
        int span=last-start;
        if(start<nbPtsUnset)
            span=_framesNoPts.at(start)-start;
        // Calculate current average frame increment from the first 8 frames at best.
        uint64_t increment=_tracks[0]._defaultFrameDuration;;
        int psize=(span<8)? span : 8;
        if(psize)
        {
            double d=_sortedPts.at(psize);
            d/=psize;
            increment=d;
        }
        // Calculate the offset we must add to all timestamps when the first frames don't have valid pts.
        uint64_t shift=increment*start;
        // Go!
        int curValid=0;
        int curNoPts=0;
        int backfill=0;
        uint64_t lastFilledDts=0;
        for(int i=0;i<last;i++)
        {
            if(curNoPts==nbPtsUnset || _framesNoPts.at(curNoPts)>i)
            {
                uint64_t dts=_sortedPts.at(curValid)+shift;
                if(backfill)
                {
                    if(curValid>1) // We don't get meaningful time increment at the start of the video, use what we alredy have.
                    {
                        double d=dts-lastFilledDts;
                        d/=backfill+1;
                        increment=d;
                    }
                    uint64_t shift2=0;
                    for(int j=0;j<backfill;j++)
                    {
                        int k=i-backfill+j;
                        shift2=increment*(j+1);
                        vid->index[k].Dts=lastFilledDts+shift2;
                    }
                    if(!shift) // We are at the frame right after early B-frames.
                    {
                        dts+=shift2;
                        if(curValid<2)
                            shift=shift2;
                    }
                    backfill=0;
                }
                vid->index[i].Dts=dts;
                lastFilledDts=dts;
                curValid++;
                if(curValid==nbValidPts) break;
            }else
            {
                backfill++;
                curNoPts++;
            }
        }
    }
    return true;
}
/**
    \fn checkDeviation
    \brief Bypass the 1ms accuracy by making sure all the frames are in the form PTS=offset + N*frameInterval
*/
bool mkvHeader::enforceFixedFrameRate(int num, int den)
{
  mkvTrak *track=_tracks;
  int nb=track->index.size();
  ADM_assert(den);
  double dHalf=(500000.*(double)num)/((double)den);
  int half=dHalf-1; // half interval in us
  int first=0;
  while(  track->index[first].Pts==ADM_NO_PTS && first<nb) first++; // we should have some at least
  uint64_t zero= track->index[first].Pts;
  {
    double dmultiple=zero+half;
    dmultiple*=den;
    dmultiple/=(1000000.*(double)num);
    zero=((uint64_t)dmultiple*1000000*num)/den;
    track->index[first].Pts=zero;
  }
  ADM_info("Num=%d Den=%d half=%d zero=%d first=%d\n",num,den,half,(int)zero,first);
  for(int i=first+1;i<nb;i++)
  {
    uint64_t pts=track->index[i].Pts;
    if(pts<zero) continue;
    pts-=zero;
    double dmultiple=(pts+half);
    dmultiple*=den;
    dmultiple/=(1000000.*(double)num);
    uint64_t multiple=(uint64_t)dmultiple;
    int64_t reconstructed=(multiple*1000000*num)/den+zero;
#if 0
    if(i<100)
    {
        pts+=zero;
        printf("frame %d multiple = %d, pts=%d, reconstructed=%d,delta = %d\n",i,(int)multiple,(int)pts,(int)reconstructed,(int)(pts-reconstructed));
    }
#endif
    track->index[i].Pts=reconstructed;
  }
  _videostream.dwScale=num;
  _videostream.dwRate=den;
  double f=num;
  f=f*1000000.;
  f/=den;
  f+=0.49;
  track->_defaultFrameDuration=f;
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


        // Search minimum and maximum between 2 frames
        // the minimum will give us the maximum fps
        // the maximum will give us the max PTS-DTS delta so that we can compute DTS
        for(int i=0;i<nb-1;i++)
        {
            if(track->index[i].flags==AVI_B_FRAME) nbBFrame++;
            if(track->index[i+1].Pts==ADM_NO_PTS || track->index[i].Pts==ADM_NO_PTS)
                continue;

            delta=(int64_t)track->index[i+1].Pts-(int64_t)track->index[i].Pts;
            if(delta<0) delta=-delta;
            if(!delta)
            {
                ADM_warning("Duplicate PTS...(%d and %d,size=%d %d)\n",i,i+1,track->index[i].size,track->index[i+1].size);
                continue;
            }
            if(delta<minDelta) minDelta=delta;
            if(delta>maxDelta) maxDelta=delta;
           // ADM_warning("Delta=%d,  MinDelta=%d MaxDelta=%d\n",delta,minDelta,maxDelta);
        }
    }
    if(nbBFrame) *bFramePresent=true;

    ADM_warning(">>> MinDelta=%d MaxDelta=%d\n",minDelta,maxDelta);
    // Check whether the default frame duration is valid
    int stdFrameRate=-1;
    if(!nbBFrame && minDelta)
    {
        bool frameDurationInvalid=false;
        if((uint64_t)maxDelta < (uint64_t)track->_defaultFrameDuration)
        {
            ADM_warning("Default frame duration is invalid, exceeds max delta\n");
            frameDurationInvalid=true;
        }
        if((uint64_t)minDelta > (uint64_t)track->_defaultFrameDuration)
        {
            ADM_warning("Default frame duration is invalid, below min delta\n");
            frameDurationInvalid=true;
        }
        if(frameDurationInvalid)
        {
            stdFrameRate=getStdFrameRate(minDelta);
            if(stdFrameRate!=-1 && stdFrameRate==getStdFrameRate(maxDelta)) // full confidence
            {
                _videostream.dwScale = candidateFrameRate[stdFrameRate].num;
                _videostream.dwRate = candidateFrameRate[stdFrameRate].den;
                track->_defaultFrameDuration = candidateFrameRate[stdFrameRate].durationInUs;
            }
        }
    }

    stdFrameRate=getStdFrameRate(track->_defaultFrameDuration);

    int num= _videostream.dwScale;
    int den= _videostream.dwRate;
    int skipped=0;
    
    int deviation=0;
    {
        // Initialize deviation
        // 
        mkvDeviation devEngine(nb);      
        int first=0;
        while(  track->index[first].Pts==ADM_NO_PTS && first<nb) 
        {
            _framesNoPts.push_back(first);
            first++; // we should have some at least
        }
        uint64_t zero= track->index[first].Pts;
        ADM_info("Num=%d Den=%d zero=%d first=%d\n",num,den,(int)zero,first);
        int valid=0;
        for(int i=first;i<nb;i++)
        {
            uint64_t pts=track->index[i].Pts;
            if(pts==ADM_NO_PTS)
            {
                _framesNoPts.push_back(i);
                continue;
            }
            if(pts<zero) // early B-frames
            {
                _framesNoPts.push_back(i);
                continue;
            }
            devEngine.add(pts-zero);
            valid++;
        }
        devEngine.sort();
        uint64_t *s = devEngine.getSorted();
        for(int i=0;i<valid;i++)
        {
            _sortedPts.push_back(s[i]-500); // subtract 500 us added by add()
        }
        ADM_info("Checking deviation for native %d %d\n", _videostream.dwScale,   _videostream.dwRate);
        deviation=devEngine.computeDeviation(  _videostream.dwScale,   _videostream.dwRate,skipped);
        
        int deviationMinDelta=100000000;
        int minDeltaSkip;
        if(minDelta)
        {
            ADM_info("Checking deviation for minDelta %d %d\n",minDelta,1000000);
            deviationMinDelta =devEngine.computeDeviation(  minDelta,1000000,minDeltaSkip   );
        }
        ADM_info("Deviation        = %d\n",deviation);
        ADM_info("DeviationMinDelta = %d\n",deviationMinDelta);
        ADM_info("Deviation skip    = %d\n",minDeltaSkip);
        bool skipValid=true;
        if(track->_defaultFrameDuration)
        {
            ADM_info("MinDelta=%d, defaultFrameDuation=%d\n",minDelta,track->_defaultFrameDuration);
            if((minDelta*2)<=track->_defaultFrameDuration)
                skipValid=false;
        }
        if(minDeltaSkip>skipped*3  && skipValid) // we are skipping a lot more frame, fps too high
        {
            ADM_info("Too many skipped frames, dropping candidates (skipped=%d, min delta skip=%d)\n",skipped,minDeltaSkip);
            deviationMinDelta=deviation*2;
        }
        if(minDelta)
        {
            bool preferMinDelta=false;
            // Hack to rescale double fps, ugly, might cause problem
            if(deviationMinDelta==deviation)
            {
                // min num/den > std num/den
                if(minDelta*_videostream.dwRate >  _videostream.dwScale*1000000*1.5)
                {
                    ADM_info("Both are equal but prefering minDelta\n");
                    preferMinDelta=true;
                }
            }
            //
            if(deviationMinDelta<deviation || preferMinDelta)
            {
                den=1000*1000;
                num=minDelta;
                deviation=deviationMinDelta;
                skipped=minDeltaSkip;
                ADM_info("Min delta is better\n");
                if(stdFrameRate==-1)
                    stdFrameRate=getStdFrameRate(minDelta);
            }
        }

        // Check std value too
        if(stdFrameRate!=-1)
        {
            int stdSkip;
            const frameRateStruct *fr=&(candidateFrameRate[stdFrameRate]);
            ADM_info("Checking deviation for stdFrameRate=%d:%d\n",fr->num,fr->den);
            int deviationStd=devEngine.computeDeviation(fr->num,fr->den,stdSkip);
            ADM_info("Deviation for stdFrameRate(%d) =%d\n",stdFrameRate,deviationStd);
            if(deviationStd<=deviation && stdSkip<=skipped*3)
            {
              num=fr->num;
              den=fr->den;
              deviation=deviationStd;
              skipped=stdSkip;
              ADM_info("Std frame rate is equal or better\n");
            }
        }
    }
    ADM_info("Old default duration    %" PRId64" us\n",track->_defaultFrameDuration);
#define TOLERANCE 500
    if(deviation < TOLERANCE)
    {
        ADM_info("We are within margin, recomputing timestamp with exact value (%d vs %d)\n",num,den);
        enforceFixedFrameRate(num,den);
        if(!skipped)
        {
            ADM_info("Enforcing constant frame rate.\n");
            _mainaviheader.dwMicroSecPerFrame=0; // perfectly regular
        }else
        {
            ADM_info("Variable frame rate with time base %d/%d\n",num,den);
            _mainaviheader.dwMicroSecPerFrame=track->_defaultFrameDuration;
        }
        // do it again, the old may not be valid 
        // Search minimum and maximum between 2 frames
        // the minimum will give us the maximum fps
        // the maximum will give us the max PTS-DTS delta so that we can compute DTS
        maxDelta=0;
        minDelta=100000000;
        for(int i=0;i<nb-1;i++)
        {
            if(track->index[i+1].Pts==ADM_NO_PTS || track->index[i].Pts==ADM_NO_PTS)
                continue;

            delta=(int64_t)track->index[i+1].Pts-(int64_t)track->index[i].Pts;
            if(delta<0) delta=-delta;
            if(!delta)
            {
                ADM_warning("Duplicate PTS...%s (%d and %d,size=%d %d)\n",ADM_us2plain(track->index[i].Pts),i,i+1,track->index[i].size,track->index[i+1].size);
                continue;
            }
            if(delta<minDelta) minDelta=delta;
            if(delta>maxDelta) maxDelta=delta;
            //printf("\/=%" PRId64" Min %" PRId64" MAX %" PRId64"\n",delta,minDelta,maxDelta);
        }

    }else
    {
        ADM_info("Variable frame rate, using 1/90000 time base.\n");
        _videostream.dwScale=1;
        _videostream.dwRate=90000;
        double f=num;
        f=f*1000000.;
        f/=den;
        f+=0.49;
        track->_defaultFrameDuration=_mainaviheader.dwMicroSecPerFrame=(uint32_t)f;
    }

    ADM_info("New default duration    %" PRId64" us\n",track->_defaultFrameDuration);
#undef TOLERANCE
    ADM_info("First frame pts     %" PRId64" us\n",track->index[0].Pts);

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
      printf("[MKV/SeekHead] Tag 0x%" PRIx64" not found (len %" PRIu64")\n",id,len);
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
      if(!father.readElemId(&id,&len))
        continue;
      if(!ADM_searchMkvTag( (MKV_ELEM_ID)id,&ss,&type))
      {
        printf("[mkvHeader::analyzeTracks] Tag 0x%" PRIx64" not found (len %" PRIu64")\n",id,len);
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
      if(!father->readElemId(&id,&len))
        continue;
      if(!ADM_searchMkvTag( (MKV_ELEM_ID)id,&ss,&type))
      {
        printf("[mkvHeader::walk] Tag 0x%" PRIx64" not found (len %" PRIu64")\n",id,len);
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
                  printf("%s:%" PRIu64"\n",ss,father->readUnsignedInt(len));
                  break;
        case ADM_MKV_TYPE_INTEGER:
                  printf("%s:%" PRId64"\n",ss,father->readSignedInt(len));
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
      if(!father->readElemId(&id,&len))
        continue;
      if(!ADM_searchMkvTag( (MKV_ELEM_ID)id,&ss,&type))
      {
        printf("[mkvHeader::walkAndFind] Tag 0x%" PRIx64" not found (len %" PRIu64")\n",id,len);
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
                  printf("%s:%" PRIu64"\n",ss,v);
                  }
                  break;
        case ADM_MKV_TYPE_INTEGER:
                  printf("%s:%" PRId64"\n",ss,father->readSignedInt(len));
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


#define FREEIF(i) { if(_tracks[i].extraData) delete [] _tracks[i].extraData; _tracks[i].extraData=0; \
                    if(_tracks[i].infoCache) delete [] _tracks[i].infoCache; _tracks[i].infoCache=0; \
                    if(_tracks[i].paramCache) delete [] _tracks[i].paramCache; _tracks[i].paramCache=0; }
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
    ADM_dealloc(_filename);
    _filename=NULL;
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

  _currentAudioTrack=0;
  _access=NULL;
  _audioStreams=NULL;

  readBuffer=NULL;
  _cuePosition=0;
  _segmentPosition=0;
  _trackPosition=0;
  _H264Recovery=16;
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
  if(!frame) *flags=AVI_KEY_FRAME | (*flags & AVI_STRUCTURE_TYPE_MASK);
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
    return _tracks[0].duration;
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

  uint32_t sz=dx->size;
  if(sz > ADM_COMPRESSED_MAX_DATA_LENGTH)
  {
        ADM_warning("Frame %u size %u exceeds max %u, truncating.\n",framenum,sz,ADM_COMPRESSED_MAX_DATA_LENGTH);
        sz = ADM_COMPRESSED_MAX_DATA_LENGTH;
  }
  img->dataLength=readAndRepeat(0,img->data, sz-3);
  ADM_assert(img->dataLength <= ADM_COMPRESSED_MAX_DATA_LENGTH);
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
 *
 * @param hd
 * @return
 */
#if 0
static int xypheLacingRead(uint8_t **hd)
{
      int x=0;
      uint8_t *p=*hd;
      while(*p==0xff)
      {
        x+=0xff;
        p++;
      }
      x+=*p;
      p++;
      *hd=p;
      return x;
}
#endif
/**
    \fn mkreformatVorbisHeader
    \brief reformat oggvorbis header to avidemux style
*/
uint8_t mkvHeader::reformatVorbisHeader(mkvTrak *trk)
{
    uint8_t *newExtra=NULL;
    int newExtraLen=0;
    bool r=ADMXiph::xiphExtraData2Adm(trk->extraData,trk->extraDataLen,&newExtra,&newExtraLen);
    if(!r)
    {
        ADM_warning("Cannot reformat vorbis extra data\n");
        return false;
    }
    // Destroy old datas
    delete [] trk->extraData;
    trk->extraData=newExtra;
    trk->extraDataLen=newExtraLen;
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
            printf("[MKV] Frame %" PRIu32" exceeds # of frames %" PRIu32"\n",frame,(uint32_t)_tracks[0].index.size());
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
            printf("[MKV] Frame %" PRIu32" exceeds # of frames %" PRIu32"\n",frame,(uint32_t)_tracks[0].index.size());
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
bool    mkvHeader::readSeekHead(ADM_ebml_file *body, uint64_t *nexthead)
{
    uint64_t vlen,len;
    ADM_info("Parsing SeekHead\n");
    if(nexthead)
        *nexthead=0;
    while(!body->finished())
    {
        if(!body->simplefind(MKV_SEEK,&vlen,false))
             break;
        ADM_ebml_file item(body,vlen);
        uint64_t id;
        ADM_MKV_TYPE type;
        const char *ss;

        if(!item.readElemId(&id,&len))
        {
            ADM_warning("Invalid data\n");
            return false;
        }
        if(!ADM_searchMkvTag( (MKV_ELEM_ID)id,&ss,&type))
        {
          printf("[MKV/SeekHead] Tag 0x%" PRIx64" not found (len %" PRIu64")\n",id,len);
          return false;
        }
        if(id!=MKV_ID)
        {
          printf("Found %s in CUES, ignored \n",ss);
          item.skip(len);
          //return false;
          continue;
        }
        // read id
        uint64_t t=item.readEBMCode_Full();
        if(!t)
        {
            ADM_warning("Invalid data\n");
            return false;
        }
        if(!ADM_searchMkvTag( (MKV_ELEM_ID)t,&ss,&type))
        {
          printf("[MKV/SeekHead] Tag 0x%" PRIx64" not found (len %" PRIu64")\n",id,len);
          return false;
        }
        ADM_info("Found entry for %s\n",ss);
        if(!item.readElemId(&id,&len))
        {
            ADM_warning("Invalid data\n");
            return false;
        }
        if(!ADM_searchMkvTag( (MKV_ELEM_ID)id,&ss,&type))
        {
          printf("[MKV/SeekHead] Tag 0x%" PRIx64" not found (len %" PRIu64")\n",id,len);
          return false;
        }
        if(id!=MKV_SEEK_POSITION)
        {
          printf("Found %s in CUES, ignored \n",ss);
          item.skip(len);
          //return false;
          continue;
        }
        uint64_t position=item.readUnsignedInt(len);
        switch(t)
        {
            case MKV_SEEK_HEAD:
                {
                    uint64_t chained=position+_segmentPosition;
                    ADM_info("Chained MKV_SEEK_HEAD at position %" PRIu64"\n",chained);
                    if(nexthead)
                        *nexthead=chained;
                    break;
                }
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

/**
 * 
 * @param trk
 * \brief  Make sure the one chunk is not too big, split it else
 *          ~ 40ms worth of data are good enough
 * @return 
 */
int mkvHeader::isBufferingNeeded(mkvTrak *trk)
{
   int n=trk->index.size();
   int max=0;
   for(int i=0;i<n;i++)
   {
       int sz=trk->index[i].size;
       if(max<sz) max=sz;
   }
   if(max<64*1024)
   {
       ADM_info("No big packet detected\n");
       return 0;
   }
   max=((max>>10)+1)<<10;
   ADM_warning("Big packet detected : %d kBytes \n",max>>10);
   return max;
}
//****************************************
//EOF
