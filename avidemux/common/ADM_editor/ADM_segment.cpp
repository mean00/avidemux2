/***************************************************************************
     \file  ADM_segment.h
     \brief Handle segment

    (C) 2002-2009 Mean, fixounet@free.Fr

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
#include "ADM_segment.h"
#include "../ADM_codecs/ADM_codec.h"
#include "ADM_image.h"
#include "../ADM_editor/ADM_edCache.h"
#include "ADM_pp.h"
#include "ADM_colorspace.h"
#include "ADM_vidMisc.h"
#include "ADM_audiocodec/ADM_audiocodec.h"
#include "ADM_script/ADM_JSif.h"

ADM_EditorSegment::ADM_EditorSegment(void)
{

}
ADM_EditorSegment::~ADM_EditorSegment()
{
    deleteAll();
}
/**
    \fn addReferenceVideo
    \brief Add a new source video, fill in the missing info + create automatically the matching seg
*/
bool        ADM_EditorSegment::addReferenceVideo(_VIDEOS *ref)
{
  uint32_t    	l;
  uint8_t 	    *d;
  aviInfo       info;
  _SEGMENT      seg;

  memset(&seg,0,sizeof(seg));

  ref->_aviheader->getVideoInfo (&info);
  ref->_aviheader->getExtraHeaderData (&l, &d);
  ref->decoder = getDecoder (info.fcc,  info.width, info.height, l, d,info.bpp);
  ref->_videoCache   =   new EditorCache(32,info.width,info.height) ;

  float frameD=info.fps1000;
  frameD=frameD/1000;
  frameD=1/frameD;
  frameD*=1000000;
  ref->timeIncrementInUs=(uint64_t)frameD;
  ADM_info("[Editor] About %"LLU" microseconds per frame\n",ref->timeIncrementInUs);
  ref->_nb_video_frames = info.nb_frames;
  //
  //  And automatically create the segment
  //
  seg._reference=videos.size();
  if(!videos.size())
  {
        seg._startTimeUs=0;
  }else
  {
      #warning todo compute cumulative time
   }
   seg._durationUs=ref->_aviheader->getVideoDuration();

    // Set the default startTime to the pts of first Pic
    vidHeader *demuxer=	ref->_aviheader;
    uint32_t flags;
    uint64_t pts,dts;
        demuxer->getFlags(0,&flags);
        demuxer->getPtsDts(0,&pts,&dts);
        ref->firstFramePts=0;
        if(pts!=ADM_NO_PTS &&pts)
        {
            ADM_warning("The first frame has a PTS >0, adjusting to %"LLU" ms\n",pts/1000);
            ref->firstFramePts=pts;
        }

    segments.push_back(seg);
    videos.push_back(*ref);
    updateStartTime();
    return true;
}
/**
    \fn deleteSegments
    \brief Empty the segments list
*/
bool        ADM_EditorSegment::deleteSegments()
{
    ADM_info("Clearing a new segment\n");
    segments.clear();
    return true;
}
/**
    \fn deleteSegments
    \brief Empty the segments list
*/
bool        ADM_EditorSegment::addSegment(_SEGMENT *seg)
{
    ADM_info("Adding a new segment\n");
    segments.push_back(*seg);
    updateStartTime();
    return true;
}

/**
	\fn deleteAll
    \brief delete datas associated with all video
*/
bool ADM_EditorSegment::deleteAll (void)
{
  ADM_info("[Editor] Deleting all video\n");
  int n=videos.size();
  for (uint32_t vid = 0; vid < n; vid++)
    {
        _VIDEOS *v=&(videos[vid]);
      // if there is a video decoder...
      if (v->decoder)
            delete v->decoder;
      if(v->color)
            delete v->color;
      if(v->_aviheader)
      {
          v->_aviheader->close ();
          delete v->_aviheader;
      }
      if(v->_videoCache)
      	delete  v->_videoCache;
      v->_videoCache=NULL;
      v->color=NULL;
      v->decoder=NULL;
      v->_aviheader=NULL;
     // Delete audio codec too
     // audioStream will be deleted by the demuxer
      if(v->audioTracks)
      {
            for(int i=0;i<v->nbAudioStream;i++)
            {
#if 0 // Deleted elsewhere ?
                if(v->audioTracks[i])
                    delete v->audioTracks[i];
#endif
                v->audioTracks[i]=NULL;
            }
            delete [] v->audioTracks;
            v->audioTracks=NULL;
      }
    }

    videos.clear();
    return true;
}


/**
    \fn resetSegment
    \brief Redo a 1:1 mapping between videos and segments
*/
bool        ADM_EditorSegment::resetSegment(void)
{
    //
    aviInfo info;
    segments.clear();
    int n=videos.size();
    for(int i=0;i<n;i++)
    {
        _SEGMENT seg;
        _VIDEOS  *vid=&(videos[i]);
        memset(&seg,0,sizeof(seg));
        seg._durationUs=vid->_aviheader->getVideoDuration();
        seg._reference=i;
        vid->_aviheader->getVideoInfo(&info);
        segments.push_back(seg);
    }
    updateStartTime();
    return true;
}
/**
    \fn getSegment
    \brief getRefVideo
*/
_SEGMENT     *ADM_EditorSegment::getSegment(int i)
{
    ADM_assert(i<segments.size());
    return &(segments[i]);
}
/**
    \fn getRefVideo
    \brief getRefVideo
*/
_VIDEOS     *ADM_EditorSegment::getRefVideo(int i)
{
    ADM_assert(i<videos.size());
    return &(videos[i]);
}
/**
    \fn getNbRefVideo
    \brief getNbRefVideo
*/
int         ADM_EditorSegment::getNbRefVideos(void)
{
    return videos.size();
}
/**
    \fn getNbSegment
    \brief  getNbSegment
*/
int         ADM_EditorSegment::getNbSegments(void)
{   
    return segments.size();
}
/**
    \fn updateStartTime
    \brief Recompute the start time of the video 
*/
bool         ADM_EditorSegment::updateStartTime(void)
{   
    int n=segments.size();
    uint64_t t=0;
    for(int i=0;i<n;i++)
    {
        segments[i]._startTimeUs=t;
        t+=segments[i]._durationUs;
    }
    // Now set the _refStartDts field
    for(int i=0;i<n;i++)
    {
        _VIDEOS *vid=getRefVideo(segments[i]._reference);
        _SEGMENT *seg=getSegment(i);

        uint64_t pts,dts;
        pts=seg->_refStartTimeUs;
        dtsFromPts(seg->_reference,pts,&dts);
        seg->_refStartDts=dts;
    }

    return true;
}
/**
    \fn getTotalDuration
    \brief 
*/
uint64_t ADM_EditorSegment::getTotalDuration(void)
{
    uint64_t dur=0;
    int n=segments.size();
    for(int i=0;i<n;i++)
        dur+=segments[i]._durationUs;
    return dur;

}
/**
    \fn getNbFrames
    \brief Weak, avoid using it
*/
uint32_t ADM_EditorSegment::getNbFrames(void)
{
    return 1;
}
/***********************************************************************/
/***********************************************************************/
/***********************************************************************/
/***********************************************************************/
/**
    \fn getRefFromTime
    \brief Return the ref video where xtime is 
*/
bool        ADM_EditorSegment::getRefFromTime(uint64_t xtime,uint32_t *refVideo)
{
    // What segments does it belong to ?
    uint32_t seg;
    uint64_t segTime;
    if(false== convertLinearTimeToSeg(  xtime, &seg, &segTime))
    {
        ADM_warning("Cannot identify segment for time %"LLU" ms\n",xtime/1000);
        return false;
    }
    _SEGMENT *s=getSegment(seg);
    ADM_assert(s);
    *refVideo=s->_reference;
    return true;
}
/**
    \fn ~ADM_audioStreamTrack
*/
 ADM_audioStreamTrack::~ADM_audioStreamTrack()
    {
        stream=NULL;
        info=NULL;   // These 2 are destroyed by the demuxer itself
        if(codec) 
        {
            delete codec;
            codec=NULL;
        }
    }

/**
    \fn convertLinearTimeToSeg
    \brief convert linear time to a segment+ offset in the segment
*/
bool        ADM_EditorSegment::convertLinearTimeToSeg(  uint64_t frameTime, uint32_t *seg, uint64_t *segTime)
{
    for(int i=0;i<segments.size();i++)
    {
        if(segments[i]._startTimeUs<=frameTime && segments[i]._startTimeUs+segments[i]._durationUs>frameTime)
        {
            *seg=i;
            *segTime=frameTime-segments[i]._startTimeUs;
            return true;
        }
    }
    int max=segments.size();
    if(max)
    {
        _SEGMENT *last=&(segments[max-1]);
        if(frameTime==last->_startTimeUs+last->_durationUs)
        {
            ADM_info("End of last segment\n");
            *seg=max-1;
            *segTime=frameTime-last->_startTimeUs;
            return true;
        }
    }
    ADM_warning("Cannot find segment matching time %"LLU"ms \n",frameTime/1000);
    return false;
}
bool        ADM_EditorSegment::convertSegTimeToLinear(  uint32_t seg,uint64_t segTime, uint64_t *frameTime)
{
    ADM_assert(seg<segments.size());
    *frameTime=segTime+segments[seg]._startTimeUs;
    return true;
}
/**
    \fn TimeToFrame
    \brief return the frameno whose PTS==time
*/
static bool TimeToFrame(_VIDEOS *v,uint64_t time,uint32_t *frame,uint32_t *oflags)
{
    vidHeader 							*demuxer=v->_aviheader;  
    int nb=demuxer->getMainHeader()->dwTotalFrames;
    for(int i=0;i<nb;i++)
    {
        uint64_t pts,dts;
        uint32_t flags;
            demuxer->getPtsDts(i,&pts,&dts);
            demuxer->getFlags(i,&flags);
            if(pts==time)
            {
                *frame=i;
                *oflags=flags;
                return true;
            }
    }
    return false;
}
/**
    \fn intraTimeToFrame
    \brief Return the frame whosePTS==seektime, assert if does not exist
*/
uint32_t    ADM_EditorSegment::intraTimeToFrame(uint32_t refVideo,uint64_t seekTime)
{
        uint32_t frame;
        uint32_t flags;
        _VIDEOS *v=getRefVideo(refVideo);
        ADM_assert(v);
        if(false==TimeToFrame(v,seekTime,&frame,&flags))
        {
            ADM_error("Cannot find frame with time %"LLU"ms\n",seekTime/1000);
            ADM_assert(0);
        }
        ADM_assert(flags & AVI_KEY_FRAME);
        return frame;
}
/**
    \fn         isKeyFrameByTime
    \brief      Return true if frame with PTS==seektime is a keyframe
*/
bool        ADM_EditorSegment::isKeyFrameByTime(uint32_t refVideo,uint64_t seekTime)
{
        uint32_t frame;
        uint32_t flags;
        _VIDEOS *v=getRefVideo(refVideo);
        ADM_assert(v);
        if(false==TimeToFrame(v,seekTime,&frame,&flags)) return false;
        if(flags & AVI_KEY_FRAME) return true;
        return false;
}
/**
    \fn removeChunk
    \brief 
*/
bool        ADM_EditorSegment::removeChunk(uint64_t from, uint64_t to)
{
    uint32_t startSeg,endSeg;
    uint64_t startOffset,endOffset;
    
    ADM_info("Cutting from %"LLU" to %"LLU" ms\n",from/1000,to/1000);
    dump();
    if(false==convertLinearTimeToSeg( from,&startSeg,&startOffset))
    {
        ADM_warning("Cannot get starting point (%"LLU" ms\n",from/1000);
        return false;
    }
    if(false==convertLinearTimeToSeg( to,&endSeg,&endOffset))
    {
        ADM_warning("Cannot get starting point (%"LLU" ms\n",from/1000);
        return false;
    }

    ADM_info("Start, seg %"LU" Offset :%"LLU" ms\n",startSeg,startOffset);
    ADM_info("End  , seg %"LU" Offset :%"LLU" ms\n",endSeg,endOffset);

    
    if(startSeg==endSeg)
    {
        // Split the seg int two..
        segments.insert(segments.begin()+startSeg+1,*getSegment(startSeg)); 
        endSeg=startSeg+1;

    }
    _SEGMENT *first=getSegment(startSeg);
      // Span over several seg...
    // 1- shorten the start segment..
  
    first->_durationUs=startOffset;
   
    // 3- Shorten last segment
    _SEGMENT *last=getSegment(endSeg);
    last->_refStartTimeUs+=endOffset;
    last->_durationUs-=endOffset;
    // 2- Kill the segment in between
    for(int i=startSeg+1;i<endSeg;i++)
    {
        segments.erase(segments.begin()+startSeg+1);
    }
    updateStartTime();
    removeEmptySegments();
    dump();
    return true;
}


/**
    \fn dump
    \brief Dump the segment content
*/
void ADM_EditorSegment::dump(void)
{
    int n=segments.size();
    printf("We have %d segments\n",n);
    for(int i=0;i<n;i++)
    {
        _SEGMENT *s=getSegment(i);
        
        jsLog(JS_LOG_NORMAL,"Segment :%d/%d",i,n);
        jsLog(JS_LOG_NORMAL,"\tReference    :%"LU,s->_reference,ADM_us2plain(s->_reference));
        jsLog(JS_LOG_NORMAL,"\tstartLinear  :%08"LLU" %s",s->_startTimeUs,ADM_us2plain(s->_startTimeUs));
        jsLog(JS_LOG_NORMAL,"\tduration     :%08"LLU" %s",s->_durationUs,ADM_us2plain(s->_durationUs));
        jsLog(JS_LOG_NORMAL,"\trefStartPts  :%08"LLU" %s",s->_refStartTimeUs,ADM_us2plain(s->_refStartTimeUs));
        jsLog(JS_LOG_NORMAL,"\trefStartDts  :%08"LLU" %s",s->_refStartDts,ADM_us2plain(s->_refStartDts));
    }
}

/**
    \fn dtsFromPts
    \brief guestimate DTS from PTS
*/
 bool        ADM_EditorSegment::dtsFromPts(uint32_t refVideo,uint64_t pts,uint64_t *dts)
{
    uint32_t frame,flags;
    _VIDEOS *vid=getRefVideo(refVideo);
    vidHeader *demuxer=vid->_aviheader;
    if(false==TimeToFrame(vid,pts,&frame,&flags))
    {
            ADM_warning("Cannot get frame with pts=%"LLU" ms\n",pts/1000);
            return false;
    }
    // No get DTS..
    int32_t deltaFrame=frame;
    uint64_t p,d;
    while(deltaFrame>0)
    {
            demuxer->getPtsDts(deltaFrame,&p,&d);
            if(d!=ADM_NO_PTS) break;
            deltaFrame--;
    }
    if(deltaFrame<0)    
    {
        ADM_warning("Cannot find a valid DTS for pts=%"LLU"ms\n",pts/1000);
        return false;
    }
    deltaFrame=frame-deltaFrame;
    *dts=d+deltaFrame*vid->timeIncrementInUs;
    return true;
}
/**
    \fn removeEmptySegments
    \brief Remove empty segments after a cut.
*/
bool        ADM_EditorSegment::removeEmptySegments(void)
{
    while(1)
    {
        int index=-1;
        int n=getNbSegments();
        for(int i=0;i<n;i++)
        {
                _SEGMENT *seg=getSegment(i);
                if(seg->_durationUs==0) index=i;
        }
        if(index==-1) break;
        segments.erase(segments.begin()+index);
    }
    return true;
}
//EOF
