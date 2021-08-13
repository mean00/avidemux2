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
#include "fourcc.h"
#include "prefs.h"

#define ADM_ZERO_OFFSET

#if 1
#define aprintf printf
#else
#define aprintf(...) {}
#endif

ADM_EditorSegment::ADM_EditorSegment(void)
{
    _sharedVideoCache = NULL;
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

    aviInfo info;
    demuxer->getVideoInfo(&info);
    uint32_t frame,flags;
    bool found = false;
    for(frame = 0; frame < info.nb_frames; frame++)
    {
        demuxer->getFlags(frame,&flags);
        if(flags & AVI_KEY_FRAME)
        {
            found = true;
            break;
        }
    }
    if(!found)
    {
        ADM_warning("No frame in ref video %d is marked as keyframe.\n",n-1);
        return false;
    }

    uint64_t pts,dts;
    demuxer->getPtsDts(frame,&pts,&dts);
    if(pts!=ADM_NO_PTS && pts >0)
    {
        ADM_warning("Updating firstFramePTS, The first keyframe has a PTS >0, adjusting to %" PRIu64" ms\n",pts/1000);
        ref->firstFramePts=pts;
    }else
    {
        ADM_info("First PTS is %s\n",ADM_us2plain(pts));
    }

    //
    n=segments.size();
    if(n)
    {
    _SEGMENT *seg=getSegment(n-1);
    uint64_t dur=ref->_aviheader->getVideoDuration();
    seg->_durationUs = dur;
#ifdef ADM_ZERO_OFFSET
    seg->_refStartTimeUs=pts;
    seg->_durationUs-=pts;
#endif
    printf("Current duration %" PRIu64" ms real one %" PRIu64" ms\n",dur/1000,seg->_durationUs/1000);
    }
    updateStartTime();
    return true;
}
/**
    \fn addReferenceVideo
    \brief Add a new source video, fill in the missing info + create automatically the matching seg
*/
bool        ADM_EditorSegment::addReferenceVideo(_VIDEOS *ref)
{
    uint32_t l = 0, cacheSize;
    uint8_t *d = NULL;
    aviInfo info;
    _SEGMENT seg;

    vidHeader *demuxer=ref->_aviheader;
    ref->dontTrustBFramePts=demuxer->unreliableBFramePts();
    demuxer->getVideoInfo (&info);
    demuxer->getExtraHeaderData (&l, &d);
    printf("[ADM_EditorSegment::addReferenceVideo] Video FCC: ");
    fourCC::print (info.fcc);
    printf("\n");
    // Printf some info about extradata
    if(l && d)
    {
        printf("[ADM_EditorSegment::addReferenceVideo] The video codec has some extradata (%d bytes)\n",l);
        mixDump(d,l);
    }
    ref->decoder = ADM_getDecoder (info.fcc, info.width, info.height, l, d, info.bpp);

    if(false==prefs->get(FEATURES_CACHE_SIZE,&cacheSize))
        cacheSize = EDITOR_CACHE_MAX_SIZE;
    if(cacheSize > EDITOR_CACHE_MAX_SIZE) cacheSize = EDITOR_CACHE_MAX_SIZE;
    if(cacheSize < EDITOR_CACHE_MIN_SIZE) cacheSize = EDITOR_CACHE_MIN_SIZE;
    // For extremely short videos like individual image files, reduce cache size to the bare minimum.
    if(info.nb_frames && info.nb_frames < cacheSize) // should we be paranoid and check the fcc?
        cacheSize = info.nb_frames + 1;

    // Shall we use a shared cache?
    bool shareCache = false;
    if(false == prefs->get(FEATURES_SHARED_CACHE,&shareCache))
        shareCache = false;
    if(shareCache)
    {
        if(!videos.size())
            _sharedVideoCache = new EditorCache(info.width,info.height);
        ref->_videoCache = _sharedVideoCache;
    }else
    {
        ref->_videoCache = new EditorCache(info.width,info.height);
    }
    ADM_assert(ref->_videoCache)
    ref->_videoCache->createBuffers(cacheSize);

    // discard implausibly high fps, hardcode the value to 25 fps
    if (info.fps1000 > 2000 * 1000)
        info.fps1000 = 25 * 1000;

    float frameD=info.fps1000;
    frameD=frameD/1000;
    frameD=1/frameD;
    frameD*=1000000;
    ref->timeIncrementInUs=(uint64_t)frameD;

    // Probe the real time increment as the value determined from FPS may be incorrect due to interlace
    uint64_t firstNonZeroDts=ADM_NO_PTS,pts,dts;
    int firstNonZeroDtsFrame;
    ADM_info("Original frame increment %s = %" PRIu64" us\n",ADM_us2plain(ref->timeIncrementInUs),ref->timeIncrementInUs);
    uint64_t minDelta=100000;
    uint64_t maxDelta=0;
    uint32_t frame,flags,fmin=0,fmax=0;
    for(frame = 0; frame < info.nb_frames; frame++)
    {
        if(!ref->fieldEncoded)
        {
            demuxer->getFlags(frame,&flags);
            if(flags & AVI_FIELD_STRUCTURE)
            {
                ADM_info("Ref video is field-encoded.\n");
                ref->fieldEncoded=true;
            }
        }
        if (demuxer->getPtsDts(frame,&pts,&dts) && dts!=ADM_NO_PTS && dts!=0)
        {
            if (firstNonZeroDts==ADM_NO_PTS)
            {
                firstNonZeroDts=dts;
                firstNonZeroDtsFrame=frame;
                continue;
            }
            if(dts>firstNonZeroDts)
            {
                uint64_t probedTimeIncrement=(dts-firstNonZeroDts)/(frame-firstNonZeroDtsFrame);
                if(probedTimeIncrement<minDelta) { minDelta=probedTimeIncrement; fmin=frame; }
                if(probedTimeIncrement>maxDelta) { maxDelta=probedTimeIncrement; fmax=frame; }
            }else if(dts==firstNonZeroDts)
            {
                ADM_warning("Duplicate DTS %s at frame %d\n",ADM_us2plain(dts),frame);
            }else
            {
                ADM_warning("DTS going back by %" PRIu64" at frame %d\n",firstNonZeroDts-dts,frame);
            }
            firstNonZeroDts=dts;
            firstNonZeroDtsFrame=frame;
        }
    }
    if(maxDelta>=minDelta)
    {
        ADM_info("min increment %s = %" PRIu64" us for frame %d\n",ADM_us2plain(minDelta),minDelta,fmin);
        ADM_info("max increment %s = %" PRIu64" us for frame %d\n",ADM_us2plain(maxDelta),maxDelta,fmax);
    }else
    {
        ADM_warning("DTS missing, cannot probe time increment.\n");
    }
  //if (minDelta==ref->timeIncrementInUs*2)
              //ref->timeIncrementInUs=minDelta;

    ADM_info("About %" PRIu64" microseconds per frame, %" PRIu32" frames\n",ref->timeIncrementInUs,info.nb_frames);
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
    seg._durationUs=demuxer->getVideoDuration();

    // Set the default startTime to the pts of first Pic
    bool found = false;
    for(frame = 0; frame < info.nb_frames; frame++)
    {
        demuxer->getFlags(frame,&flags);
        if(flags & AVI_KEY_FRAME)
        {
            found = true;
            break;
        }
    }
    if(!found)
    {
        ADM_warning("Reached the end of ref video while searching for the first keyframe.\n");
        if(!videos.size())
            delete ref->_videoCache;
        ref->_videoCache = NULL;
        if(ref->decoder) delete ref->decoder;
        ref->decoder = NULL;
        return false;
    }
    demuxer->getPtsDts(frame,&pts,&dts);
    ref->firstFramePts=0;
    if(pts==ADM_NO_PTS) ADM_warning("First frame has unknown PTS\n");
    if(dts==ADM_NO_PTS) ADM_warning("The first frame DTS is not set\n");
    else ADM_info("The first frame DTS = %" PRIu64" ms\n",dts/1000);
    if(pts!=ADM_NO_PTS &&pts)
    {
        ADM_warning("The first frame has a PTS > 0, adjusting to %" PRIu64" %s\n",
                (pts>=1000)? pts/1000 : pts,
                (pts>=1000)? "ms" : "us");
        ref->firstFramePts=pts;
#ifdef ADM_ZERO_OFFSET
        seg._refStartTimeUs=pts;
        seg._durationUs-=pts;
#endif
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
    \fn addSegment
    \brief Add a segment to the segments list
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
    ADM_info("[Editor] Deleting all videos\n");
    // Delete cache 1st, might contain refs to decoder etc..
    if(_sharedVideoCache)
        delete _sharedVideoCache;
    for (uint32_t vid = 0; vid < videos.size(); vid++)
    {
        _VIDEOS *v=&(videos[vid]);
        if(v->_videoCache && v->_videoCache != _sharedVideoCache) // per-video cache
            delete v->_videoCache;
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
        if(v->infoCache)
            delete [] v->infoCache;
        if(v->paramCache)
            delete [] v->paramCache;
        v->_videoCache=NULL;
        v->color=NULL;
        v->decoder=NULL;
        v->_aviheader=NULL;
        v->infoCache=NULL;
        v->paramCache=NULL;
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
    _sharedVideoCache = NULL;
    clipboard.clear();
    videos.clear();
    segments.clear();
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

        seg._durationUs=vid->_aviheader->getVideoDuration();
#ifdef ADM_ZERO_OFFSET
        uint64_t pts=vid->firstFramePts;
        if(pts!=ADM_NO_PTS && pts)
        {
            ADM_warning("The first frame in ref video %d has PTS = %" PRIu64" us, adjusting the segment\n",i,pts);
            seg._refStartTimeUs=pts;
            seg._durationUs-=pts;
        }
#endif
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
    \fn getSegments
    \brief getter for the list of segments
*/
ListOfSegments ADM_EditorSegment::getSegments(void)
{
    if(segments.empty()) ADM_assert(0);
    ListOfSegments segm=segments;
    return segm;
}
/**
    \fn setSegments
    \brief setter for the list of segments
*/
bool ADM_EditorSegment::setSegments(ListOfSegments segm)
{
    if(segm.size())
    {
        segments=segm;
        return true;
    }
    return false;
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

    uint64_t cumulated = 0;

    for(int i=0;i<n;i++)
    {
        _VIDEOS *vid=getRefVideo(segments[i]._reference);
        _SEGMENT *seg=getSegment(i);
        vidHeader *demuxer=vid->_aviheader;

        aviInfo info;
        demuxer->getVideoInfo(&info);

        uint64_t pts,dts;
        const uint64_t compare = pts = seg->_refStartDts = seg->_refStartTimeUs;
        // Skip to the first keyframe
        uint32_t frame,flags;
        bool found = false;
        for(frame = 0; frame < info.nb_frames; frame++)
        {
            demuxer->getFlags(frame,&flags);
            if(flags & AVI_KEY_FRAME)
            {
                found = true;
                break;
            }
        }
        if(!found)
        {
            ADM_error("No frame in ref %u is flagged as keyframe, crashing.\n",segments[i]._reference);
            ADM_assert(0);
        }
        // Special case : If pts=0 it might mean beginning of seg i, but the PTS might be not 0
        // in such a case the DTS is wrong
        uint64_t pts2,dts2,start = pts;
        // We need to ensure that start time in ref matches a frame exactly
        int depth = 32; // H.264 max ref frames * 2 fields
        uint64_t candidate = ADM_NO_PTS;
        for(uint32_t i = frame; i < info.nb_frames; i++)
        {
            demuxer->getPtsDts(i,&pts2,&dts2);
            if(pts2 == ADM_NO_PTS)
                continue;
            if(i == frame)
            {
                start = pts2;
                if(pts < start)
                {
                    pts = start;
                    break;
                }
            }
            if(pts2 < start)
                continue;
            if(pts2 == pts)
            {
                candidate = ADM_NO_PTS;
                break;
            }
            if(pts2 > pts)
            {
                if(candidate == ADM_NO_PTS)
                {
                    candidate = pts2;
                    continue;
                }
                if(pts2 < candidate)
                    candidate = pts2;
                if(depth-- < 0) break;
            }
        }
        if(candidate != ADM_NO_PTS)
            pts = candidate;
#ifdef ADM_ZERO_OFFSET
        if(pts != compare)
        {
            char *old = ADM_strdup(ADM_us2plain(compare));
            ADM_warning("Updating start time in ref from %s to %s\n",old,ADM_us2plain(pts));
            ADM_dealloc(old);
            // adjust duration
            uint64_t delta = (pts > compare)? pts - compare : compare - pts;
            if(pts > compare)
                seg->_durationUs = (seg->_durationUs > delta)? seg->_durationUs - delta : 0;
            else
                seg->_durationUs += delta;
        }
        // adjust start time in ref
        seg->_refStartTimeUs = pts;
#endif
        // set linear start time
        seg->_startTimeUs = cumulated;
        cumulated += seg->_durationUs;
        // set the _refStartDts field
        if(dtsFromPts(seg->_reference,pts,&dts))
        {
            ADM_info("Setting DTS start in ref to %s\n",ADM_us2plain(dts));
            seg->_refStartDts = dts;
        }
    }
    ADM_info("New total duration = %s\n",ADM_us2plain(cumulated));
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
    if(extraCopy)
    {
        delete [] extraCopy;
        extraCopy=NULL;
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
        //ADM_info("Frame time=0, taking first segment \n");
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
    removeEmptySegments();
    updateStartTime();
    if(isEmpty())
    {
        GUI_Error_HIG(QT_TRANSLATE_NOOP("adm","Error"),QT_TRANSLATE_NOOP("adm","You cannot remove *all* the video\n"));
        segments=tmp;
        updateStartTime();
        return false;

    }
    dump();
    return true;
}
/**
    \fn truncateVideo
    \brief Remove a part of the video from the given time to the end
*/
bool ADM_EditorSegment::truncateVideo(uint64_t from)
{
    uint32_t startSeg;
    uint64_t startOffset;

    ADM_info("Truncating from %" PRIu64" ms\n",from/1000);
    dump();
    if(false==convertLinearTimeToSeg(from,&startSeg,&startOffset))
    {
        ADM_warning("Cannot get starting point for linear time %" PRIu64" ms\n",from/1000);
        return false;
    }

    ADM_info("Start in segment %" PRIu32" at offset :%" PRIu64" ms\n",startSeg,startOffset);
    ListOfSegments tmp=segments;

    _SEGMENT *first=getSegment(startSeg);
    _VIDEOS *vid=getRefVideo(first->_reference);
    if(startSeg && vid->firstFramePts >= startOffset)
    { // "from" matches the first frame of the ref video
        first->_durationUs=0; // kill this segment
        ADM_info("Removing the whole segment.\n");
    }else
    { // shorten the start segment, the frame at "from" is dropped
        first->_durationUs=startOffset; // +vid->timeIncrementInUs;
    }
    // remove following segments
    int n=segments.size();
    for(int i=startSeg+1;i<n;i++)
    {
        segments.erase(segments.begin()+startSeg+1);
    }
    removeEmptySegments();
    updateStartTime();
    if(isEmpty())
    {
        GUI_Error_HIG(QT_TRANSLATE_NOOP("adm","Error"),QT_TRANSLATE_NOOP("adm","You cannot remove *all* the video\n"));
        segments=tmp;
        updateStartTime();
        return false;
    }
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
    d+=deltaFrame*vid->timeIncrementInUs;
    if(d>pts)
    {
        ADM_warning("Calculated DTS=%" PRIu64" > PTS=%" PRIu64"\n",d,pts);
        *dts=pts;
        return false;
    }
    *dts=d;
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
#ifndef ADM_ZERO_OFFSET
                if(seg->_refStartTimeUs==0 && seg->_durationUs==vid->firstFramePts) index=i;
#endif
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
    if(clipboardEmpty())
    {
        ADM_info("The clipboard is empty, nothing to do\n");
        return true;
    }
    ADM_info("Pasting from clipboard to %s\n",ADM_us2plain(currentTime));
    uint32_t startSeg;
    uint64_t startSegTime;
    convertLinearTimeToSeg(  currentTime, &startSeg,&startSegTime);    
    ListOfSegments tmp=segments;
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
    // If a video doesn't start at zero and we paste to its first frame,
    // we end up with an empty segment at the beginning. Remove it.
    removeEmptySegments();
    updateStartTime();
    dump();
    return true;
}

/**
 * \fn appendFromClipBoard
 * \brief append clipboard at the end of video
 * @return 
 */
bool ADM_EditorSegment::appendFromClipBoard(void)
{
    if(clipboardEmpty())
    {
        ADM_info("The clipboard is empty, nothing to do\n");
        return true;
    }
    ADM_info("Appending from clipboard\n");
    ListOfSegments tmp=segments;
    for(int i=0;i<clipboard.size();i++) segments.push_back(clipboard[i]);
    updateStartTime();
    dump();
    return true;
}

/**
 * \fn clipboardEmpty
 */
bool ADM_EditorSegment::clipboardEmpty(void)
{
    return clipboard.empty();
}

//EOF
