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
#include "ADM_cpp.h"
#include "ADM_default.h"
#include "ADM_segment.h"
#include "ADM_codec.h"
#include "ADM_image.h"
#include "ADM_edCache.h"
#include "ADM_pp.h"
#include "ADM_colorspace.h"
#include "ADM_vidMisc.h"
#include "ADM_audiocodec.h"
#include "ADM_codec.h"
#include "DIA_coreToolkit.h"
#include "ADM_vidMisc.h"

#if 1
#define aprintf printf
#else
#define aprintf(...) {}
#endif

ADM_EditorSegment::ADM_EditorSegment(void)
{
}
ADM_EditorSegment::~ADM_EditorSegment()
{
    deleteAll();
}
/**
    \fn updateRefVideo
    \brief Update start time
*/
bool        ADM_EditorSegment::halfFps(void)
{
    int n=videos.size();
    ADM_assert(n);
    _VIDEOS *ref=getRefVideo(n-1);
    ref->timeIncrementInUs*=2;
    ref->_aviheader->getVideoStreamHeader()->dwRate/=2;
    return true;
}
/**
    \fn updateRefVideo
    \brief Update start time
*/
bool        ADM_EditorSegment::updateRefVideo(void)
{
    int n=videos.size();
    ADM_assert(n);
    _VIDEOS *ref=getRefVideo(n-1);
    vidHeader *demuxer=ref->_aviheader;
    uint64_t pts,dts;

        demuxer->getPtsDts(0,&pts,&dts);
        if(pts!=ADM_NO_PTS && pts >0)
        {
            ADM_warning("Updating firstFramePTS, The first frame has a PTS >0, adjusting to %" PRIu64" ms\n",pts/1000);
            ref->firstFramePts=pts;
        }else
        {
            ADM_info("First PTS is %s\n",ADM_us2plain(pts));
        }

    updateStartTime();
    //
    n=segments.size();
    if(n)
    {
    _SEGMENT *seg=getSegment(n-1);
    uint64_t dur=ref->_aviheader->getVideoDuration();
    printf("Current duration %" PRIu64" ms real one %" PRIu64" ms\n",dur/1000,seg->_durationUs/1000);
    }

    return true;
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

  ref->dontTrustBFramePts=ref->_aviheader->unreliableBFramePts();
  ref->_aviheader->getVideoInfo (&info);
  ref->_aviheader->getExtraHeaderData (&l, &d);
  ref->decoder = ADM_getDecoder (info.fcc,  info.width, info.height, l, d,info.bpp);
  ref->_videoCache   =   new EditorCache(8,info.width,info.height) ;

  float frameD=info.fps1000;
  frameD=frameD/1000;
  frameD=1/frameD;
  frameD*=1000000;
  ref->timeIncrementInUs=(uint64_t)frameD;

  // Probe the real time increment as the value determined from FPS may be incorrect due to interlace
  uint64_t firstNonZeroDts=ADM_NO_PTS,pts,dts;
  int firstNonZeroDtsFrame;
  ADM_info("[editor] Original frame increment %s\n",ADM_us2plain(ref->timeIncrementInUs));
  uint64_t minDelta=100000;
  uint64_t maxDelta=0;
  for (int frame=0; frame<info.nb_frames; frame++)
  {
      if (ref->_aviheader->getPtsDts(frame,&pts,&dts) && dts!=ADM_NO_PTS && dts!=0)
      {
          if (firstNonZeroDts==ADM_NO_PTS)
          {
              firstNonZeroDts=dts;
              firstNonZeroDtsFrame=frame;
              continue;
          }

          uint64_t probedTimeIncrement=(dts-firstNonZeroDts)/(frame-firstNonZeroDtsFrame);
          if(probedTimeIncrement<minDelta) minDelta=probedTimeIncrement;
          if(probedTimeIncrement>maxDelta) maxDelta=probedTimeIncrement;
          firstNonZeroDts=dts;
          firstNonZeroDtsFrame=frame;
      }
  }
  ADM_info("[Editor] min increment %s\n",ADM_us2plain(minDelta));
  ADM_info("[Editor] max increment %s\n",ADM_us2plain(maxDelta));
  
  //if (minDelta==ref->timeIncrementInUs*2)
              //ref->timeIncrementInUs=minDelta;

  
  ADM_info("[Editor] About %" PRIu64" microseconds per frame\n",ref->timeIncrementInUs);
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
//      #warning todo compute cumulative time
   }
   seg._durationUs=ref->_aviheader->getVideoDuration();

    // Set the default startTime to the pts of first Pic
    vidHeader *demuxer=	ref->_aviheader;
    uint32_t flags;
        demuxer->getFlags(0,&flags);
        demuxer->getPtsDts(0,&pts,&dts);
        ref->firstFramePts=0;
        if(pts==ADM_NO_PTS) ADM_warning("First frame has unknown PTS\n");
        if(pts!=ADM_NO_PTS &&pts)
        {
            ADM_warning("The first frame has a PTS >0, adjusting to %" PRIu64" ms\n",pts/1000);
            ref->firstFramePts=pts;
        }

    if(!segments.empty()) undoSegments.push_back(segments);

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
    undoSegments.clear();
    return true;
}
/**
    \fn deleteSegments
    \brief Empty the segments list
*/
bool        ADM_EditorSegment::addSegment(_SEGMENT *seg)
{
    ADM_info("Adding a new segment\n");
    undoSegments.push_back(segments);
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
  ADM_info("[Editor] Deleting all videos\n");
  int n=videos.size();
  for (uint32_t vid = 0; vid < n; vid++)
    {
        _VIDEOS *v=&(videos[vid]);
        // Delete cache 1st, might contain refs to decoder etc..
      if(v->_videoCache)
      	delete  v->_videoCache;
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
    
      v->_videoCache=NULL;
      v->color=NULL;
      v->decoder=NULL;
      v->_aviheader=NULL;
     // Delete audio codec too
     // audioStream will be deleted by the demuxer


            int nb=v->audioTracks.size();
            for(int i=0;i<nb;i++)
            {

                ADM_audioStreamTrack *t=v->audioTracks[i];
                v->audioTracks[i]=NULL;
#if 1 // Deleted elsewhere ?
                if(t)
                    delete t;
#endif
            }
            v->audioTracks.clear();


    }

    videos.clear();
    segments.clear();
    undoSegments.clear();
    return true;
}


/**
    \fn undo
    \brief
*/
bool        ADM_EditorSegment::undo(void)
{
    if(undoSegments.empty()) return false;
    clipboard.clear();
    segments=undoSegments.back(); 
    undoSegments.pop_back();
    updateStartTime();
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
    undoSegments.clear();
    int n=videos.size();
    for(int i=0;i<n;i++)
    {
        _SEGMENT seg;
        _VIDEOS  *vid=&(videos[i]);

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
    int sz=segments.size();
    if(i>=sz)
    {
        ADM_info("Request for segment out of range : %d / %d\n",i,sz);
        ADM_assert(0);
    }
    return &(segments[i]);
}
/**
    \fn getRefVideo
    \brief getRefVideo
*/
_VIDEOS     *ADM_EditorSegment::getRefVideo(int i)
{
    int sz=videos.size();
    if(i>=sz)
    {
        ADM_info("Request for video out of range : %d / %d\n",i,sz);
        ADM_assert(0);
    }
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
    if(!n) return true;
    
    uint64_t t=0;
    t=segments[0]._startTimeUs;
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
        vidHeader *demuxer=vid->_aviheader;


        uint64_t pts,dts;
        pts=seg->_refStartTimeUs;
        // Special case : If pts=0 it might mean beginning of seg i, but the PTS might be not 0
        // in such a case the DTS is wrong
        if(!pts)
        {
            uint64_t pts2,dts2;
            demuxer->getPtsDts(0,&pts2,&dts2);
            if(pts2!=ADM_NO_PTS)
            {
                ADM_info("Using pts2=%s to get dts, as this video does not start at zero\n",ADM_us2plain(pts2));
                pts=pts2;
            }

        }
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
        ADM_warning("Cannot identify segment for time %" PRIu64" ms\n",xtime/1000);
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
    if(!frameTime && segments.size()) // pick the first one
    {
        ADM_info("Frame time=0, taking first segment \n");
        *seg=0;
        *segTime=0; // ??
        return true;
    }
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
    ADM_warning("Cannot find segment matching time %" PRIu64"ms \n",frameTime/1000);
    dump();
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
    vidHeader *demuxer=v->_aviheader;
    bool warn=false;
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
            if(dts!=ADM_NO_PTS && warn==false)
            {
                if(dts>time)
                {
                    ADM_error("We reached frame %d with a PTS of %" PRIu64" when looking for PTS %" PRIu64"\n",
                                            i,dts,time);
                    warn=true;
                }
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
            ADM_error("Cannot find frame with time %" PRIu64"ms\n",seekTime/1000);
            ADM_assert(0);
        }
        uint32_t next;
        v->_aviheader->getFlags(frame+1,&next);
        if(!((next | flags) & AVI_KEY_FRAME)) // The 2nd field might be keyframe
        {
                ADM_warning("Seeking to a non keyframe (time=%s), flags=%x, flagsNext=%x\n",ADM_us2plain(seekTime),flags,next);
                ADM_warning("This is not normal unless you start frame is not a keyframe\n");
        }
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

    ADM_info("Cutting from %" PRIu64" to %" PRIu64" ms\n",from/1000,to/1000);
    dump();
    if(false==convertLinearTimeToSeg( from,&startSeg,&startOffset))
    {
        ADM_warning("Cannot get starting point (%" PRIu64" ms\n",from/1000);
        return false;
    }
    if(false==convertLinearTimeToSeg( to,&endSeg,&endOffset))
    {
        ADM_warning("Cannot get end point (%" PRIu64" ms\n",to/1000);
        return false;
    }

    ADM_info("Start, seg %" PRIu32" Offset :%" PRIu64" ms\n",startSeg,startOffset);
    ADM_info("End  , seg %" PRIu32" Offset :%" PRIu64" ms\n",endSeg,endOffset);
    ListOfSegments tmp=segments;
    

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
    if(isEmpty())
    {
        GUI_Error_HIG(QT_TRANSLATE_NOOP("adm","Error"),QT_TRANSLATE_NOOP("adm","You cannot remove *all* the video\n"));
        segments=tmp;
        updateStartTime();
        return false;

    }
    undoSegments.push_back(tmp);
    dump();
    return true;
}


/**
    \fn dump
    \brief Dump the segment content
*/
void ADM_EditorSegment::dump(void)
{
    dumpSegmentsInternal(segments);
}

void ADM_EditorSegment::dumpSegmentsInternal(ListOfSegments &l)
{
    int n=l.size();
    for(int i=0;i<n;i++)
    {
        _SEGMENT s=l.at(i);

        printf("Segment :%d/%d\n",i,n);
        printf("\tReference    :%" PRIu32"    %s\n",s._reference,ADM_us2plain(s._reference));
        printf("\tstartLinear  :%08" PRIu64" %s\n",s._startTimeUs,ADM_us2plain(s._startTimeUs));
        printf("\tduration     :%08" PRIu64" %s\n",s._durationUs,ADM_us2plain(s._durationUs));
        printf("\trefStartPts  :%08" PRIu64" %s\n",s._refStartTimeUs,ADM_us2plain(s._refStartTimeUs));
        printf("\trefStartDts  :%08" PRIu64" %s\n",s._refStartDts,ADM_us2plain(s._refStartDts));
    }
}

void       ADM_EditorSegment::dumpSegment(int i)
{
    int n=segments.size();
    if(i>=n)
    {
        ADM_error("Segment %d too big (%d)\n",i,(int)n);
        return ;
    }
        _SEGMENT *s=getSegment(i);

        printf("Segment :%d/%d\n",i,n);
        printf("\tReference    :%" PRIu32"    %s\n",s->_reference,ADM_us2plain(s->_reference));
        printf("\tstartLinear  :%08" PRIu64" %s\n",s->_startTimeUs,ADM_us2plain(s->_startTimeUs));
        printf("\tduration     :%08" PRIu64" %s\n",s->_durationUs,ADM_us2plain(s->_durationUs));
        printf("\trefStartPts  :%08" PRIu64" %s\n",s->_refStartTimeUs,ADM_us2plain(s->_refStartTimeUs));
        printf("\trefStartDts  :%08" PRIu64" %s\n",s->_refStartDts,ADM_us2plain(s->_refStartDts));
}
/**
    \fn dumpRefVideos
    \brief Dump the refVideo content
*/
void ADM_EditorSegment::dumpRefVideos(void)
{

    int n=videos.size();
    printf("We have %d reference videos\n",n);
    for(int i=0;i<n;i++)
    {
        _VIDEOS *s=getRefVideo(i);

        printf("Videos :%d/%d\n",i,n);
        printf("\tfirstFramePts      :%08" PRIu64" %s\n",s->firstFramePts,ADM_us2plain(s->firstFramePts));
        printf("\ttimeIncrementInUs  :%08" PRIu64" %s\n",s->timeIncrementInUs,ADM_us2plain(s->timeIncrementInUs));
        printf("\tnb frames    :%04" PRIu32"\n",s->_nb_video_frames);
    }

}


/**
    \fn dtsFromPts
    \brief guestimate DTS from PTS. For the wanted frame, we go back until we find a valid DTS
                then the wanted DTS=~ valid DTS + timeIncrement * number of frames in between
*/
 bool        ADM_EditorSegment::dtsFromPts(uint32_t refVideo,uint64_t pts,uint64_t *dts)
{
    uint32_t frame,flags;
    _VIDEOS *vid=getRefVideo(refVideo);
    vidHeader *demuxer=vid->_aviheader;
    if(false==TimeToFrame(vid,pts,&frame,&flags))
    {
            ADM_warning("Cannot get frame with pts=%" PRIu64" ms\n",pts/1000);
            return false;
    }
    // Now get DTS..
    uint64_t p,d;
    if(!frame) // The very first frame
    {
        demuxer->getPtsDts(0,&p,&d);
        if(d==ADM_NO_PTS)
        {
            ADM_warning("No DTS available for first frame, putting pts, probably incorrect\n");
            *dts=pts;
        }else
        {
            *dts=d;
        }
        return true;
    }
    int32_t deltaFrame=frame;


    while(deltaFrame>0)
    {
            demuxer->getPtsDts(deltaFrame,&p,&d);
            if(d!=ADM_NO_PTS) break;
            deltaFrame--;
    }
    if(deltaFrame<0)
    {
        ADM_warning("Cannot find a valid DTS for pts=%" PRIu64"ms\n",pts/1000);
        *dts=pts;
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
                _VIDEOS  *vid=this->getRefVideo(seg->_reference);
                if(seg->_durationUs==0) index=i;
                if(seg->_refStartTimeUs==0 && seg->_durationUs==vid->firstFramePts) index=i; 
        }
        if(index==-1) break;
        segments.erase(segments.begin()+index);
    }
    return true;
}
/**
 * \fn isEmpty
 * @return 
 */
bool        ADM_EditorSegment::isEmpty(void)
{
    if(segments.size()==0) return true;
    return false;
            
}
/**
    \fn LinearToRefTime
    \brief Convert linear time to the time in the ref video

*/
bool        ADM_EditorSegment::LinearToRefTime(int segNo,uint64_t linear,uint64_t *refTime)
{
    _SEGMENT *seg=getSegment(segNo);
    ADM_assert(seg);
    if(linear<seg->_startTimeUs)
    {
        ADM_warning("This given time is not in the segment: Given time %" PRIu64", seg start at %" PRIu64"\n",
                        linear, seg->_startTimeUs);
    }
    if(linear>=seg->_startTimeUs+seg->_durationUs)
    {
        ADM_warning("This given time is not in the segment: Given time %" PRIu64", seg end at %" PRIu64"\n",
                        linear, seg->_startTimeUs+seg->_durationUs);

    }
    int64_t time=(int64_t)seg->_refStartTimeUs+(int64_t)linear;
    time-=(int64_t)seg->_startTimeUs;
    if(time<0) return false;
    *refTime=(uint64_t )time;
    return true;
}
/**
 * \fn copyToClipBoard
 * \Brief copy the section between startTime and endTime to clipboard
 * @param startTime
 * @param endTime
 * @return 
 */
bool        ADM_EditorSegment::copyToClipBoard(uint64_t startTime, uint64_t endTime)
{
    ADM_info("Copy to clipboard from %s",ADM_us2plain(startTime));
    ADM_info("to %s\n",ADM_us2plain(endTime));
    uint32_t startSeg,endSeg;
    uint64_t startSegTime,endSegTime;
    convertLinearTimeToSeg(  startTime, &startSeg,&startSegTime);
    convertLinearTimeToSeg(  endTime, &endSeg,&endSegTime);
    dump();
    clipboard.clear();
    for(int seg=startSeg;seg<=endSeg;seg++)
    {
        _SEGMENT s=segments[seg],s2=s;
        aprintf("Adding segment %d to clipboard\n",seg);
        if(s._startTimeUs<=startTime && (s._startTimeUs+s._durationUs)>startTime)
        {
            // need to refine 1st seg

            uint64_t offset=startTime-s._startTimeUs;
            s._refStartTimeUs+=offset;
            s._durationUs-=offset;         // take into account the part we chopped
            s._startTimeUs+=offset;
            aprintf("Marker A is here offset=%" PRIu64"\n",offset);
        }
        if(s2._startTimeUs<=endTime && (s2._startTimeUs+s2._durationUs)>=endTime)
        {            
            // need to refine last seg            
            uint64_t offset=endTime-s2._startTimeUs;
            s._durationUs=endTime-s._startTimeUs;
            aprintf("Marker B is here offset=%" PRIu64"\n",offset);
        }
        // TODO refine timing for 1st/last/duration/...
        clipboard.push_back(s);        
    }
    dumpClipBoard();
    return false;
}
/**
 * \fn dumpClipBoard
 * @return 
 */
bool        ADM_EditorSegment::dumpClipBoard()
{
    dumpSegmentsInternal(clipboard);
    return true;
}

/**
 * \fn pasteFromClipBoard
 * \brief instert clipboard at currentTime position
 * @param currentTime
 * @return 
 */
bool        ADM_EditorSegment::pasteFromClipBoard(uint64_t currentTime)
{
    ADM_info("Pasting from clipboard to %s\n",ADM_us2plain(currentTime));
    uint32_t startSeg;
    uint64_t startSegTime;
    convertLinearTimeToSeg(  currentTime, &startSeg,&startSegTime);    
    ListOfSegments newSegs;
    int n=segments.size();
    for(int i=0;i<n;i++)
    {
        _SEGMENT s=segments[i];
        if(i==startSeg)
        {
            // insert clipboard
            // Do we need to split it ?
            if(currentTime==s._startTimeUs)
            {
                // nope
                for(int j=0;j<clipboard.size();j++) newSegs.push_back(clipboard[j]);
            }else
            {
                 _SEGMENT pre=s,post=s;
                 uint64_t offset=currentTime-s._startTimeUs;
                 pre._durationUs=offset;
                 post._refStartTimeUs+=offset;
                 post._durationUs-=offset;
                 newSegs.push_back(pre);
                 for(int j=0;j<clipboard.size();j++) newSegs.push_back(clipboard[j]);
                 newSegs.push_back(post);
                 continue;
            }
                    
        }
        newSegs.push_back(s);
    }
    segments=newSegs;
    updateStartTime();
    return true;
}

//EOF
