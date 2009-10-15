/***************************************************************************
                          ADM_edSearch.cpp  -  description
                             -------------------
    begin                : Sat Apr 13 2002
    copyright            : (C) 2002 by mean
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
#include "fourcc.h"
#include "ADM_editor/ADM_edit.hxx"

/**
    \fn getNKFramePTS
*/
bool	ADM_Composer::getNKFramePTS(uint64_t *frameTime)
{
uint64_t refTime,nkTime,segTime;
int lastSeg=_segments.getNbSegments();
uint32_t seg;
bool r;
    // 1- Convert frameTime to segments
    if(false== _segments.convertLinearTimeToSeg(  *frameTime, &seg, &segTime))
    {
        ADM_warning(" Cannot find seg for time %"LLD"\n",*frameTime);
        return false;
    }   
    // 
again:
    _SEGMENT *s=_segments.getSegment(seg);
    uint32_t ref=s->_reference;
    // 2- Now search the previous keyframe in the ref image...
    // The time in reference = relTime+segmentStartTime
    refTime=s->_refStartTimeUs+segTime; // Absolute time in the reference image
    
    r=searchNextKeyFrameInRef(ref,refTime,&nkTime);

    // 3- if it does not belong to the same seg  ....
    if(r==false || nkTime > (s->_refStartTimeUs+s->_durationUs))
    {
        if(seg>=lastSeg-1)
        {
            ADM_warning(" No next keyframe keyfr for frameTime \n");
            return false;
        }
        // Go to the next segment
        seg++;
        goto again;
    }
    // Gotit, now convert it to the linear time
    // We have the time in the ref video, convert it to relatative to this segment
    nkTime-=s->_refStartTimeUs;  // Ref to segment...
    _segments.convertSegTimeToLinear(seg,nkTime,frameTime);
    return true;

}

/**
    \fn getPKFramePTS
*/

bool			ADM_Composer::getPKFramePTS(uint64_t *frameTime)
{
uint64_t refTime,nkTime,segTime;
int lastSeg=_segments.getNbSegments();
uint32_t seg;
bool r;
    // 1- Convert frameTime to segments
    if(false== _segments.convertLinearTimeToSeg(  *frameTime, &seg, &segTime))
    {
        ADM_warning(" Cannot find seg for time %"LLD"\n",*frameTime);
        return false;
    }   
    // 
again:
    _SEGMENT *s=_segments.getSegment(seg);
    uint32_t ref=s->_reference;
    // 2- Now search the previous keyframe in the ref image...
    // The time in reference = relTime+segmentStartTime
    refTime=s->_refStartTimeUs+segTime; // Absolute time in the reference image
    
    r=searchPreviousKeyFrameInRef(ref,refTime,&nkTime);

    // 3- if it does not belong to the same seg  ....
    if(r==false || nkTime < (s->_refStartTimeUs))
    {
        if(!seg)
        {
            ADM_warning(" No next previous keyfr for frameTime %"LLU"\n",*frameTime);
            return false;
        }
        // Go to the next segment
        seg--;
        goto again;
    }
    // Gotit, now convert it to the linear time
    nkTime-=s->_refStartTimeUs;  // Ref to segment...
    _segments.convertSegTimeToLinear(seg,nkTime,frameTime);
    return true;
}
/**
    \fn searchNextKeyFrameInRef
    \brief Search next key frame in ref video ref
    @param ref: # of ref video
    @param refTime : PTS to search keyframe after   
    @param nkTime : Time of the ref video

*/
bool ADM_Composer::searchNextKeyFrameInRef(int ref,uint64_t refTime,uint64_t *nkTime)
{
    // Search from the end till we get a keyframe
    _VIDEOS *v=_segments.getRefVideo(ref);
    uint32_t nbFrame=v->_nb_video_frames;
    uint64_t pts,dts;
    for(int i=0;i<nbFrame;i++)
    {
        uint64_t p;
        uint32_t flags;
        v->_aviheader->getFlags(i,&flags);
        if(!(flags & AVI_KEY_FRAME)) continue;
        v->_aviheader->getPtsDts(i,&pts,&dts);
        if(pts==ADM_NO_PTS) continue;
        if(pts>refTime)
        {
            *nkTime=pts;
            return true;
        }
    }
    return false;
}
/**
    \fn searchPreviousKeyFrameInRef
    \brief Search next key frame in ref video ref
    @param ref: # of ref video
    @param refTime : PTS to search keyframe after   
    @param nkTime : Time of the ref video

*/
bool ADM_Composer::searchPreviousKeyFrameInRef(int ref,uint64_t refTime,uint64_t *nkTime)
{
    // Search from the end till we get a keyframe
    _VIDEOS *v=_segments.getRefVideo(ref);
    uint32_t nbFrame=v->_nb_video_frames;
    uint64_t pts,dts;
    for(int i=nbFrame-1;i>=0;i--)
    {
        uint64_t p;
        uint32_t flags;
        v->_aviheader->getFlags(i,&flags);
        if(!(flags & AVI_KEY_FRAME)) continue;
        v->_aviheader->getPtsDts(i,&pts,&dts);
        if(pts==ADM_NO_PTS) continue;
        if(pts<refTime)
        {
            *nkTime=pts;
            return true;
        }
    }
    return false;
}

/**
    \fn searchFrameBefore
    \brief Return the frame number with pts just before pts
*/
uint32_t ADM_Composer::searchFrameBefore(uint64_t pts)
{
uint64_t refTime;
uint32_t ref;
    if(false==_segments.getRefFromTime(pts,&ref,&refTime))
    {
        ADM_warning(" Failed for pts %"LLU"\n",pts);
        ref=0;
        refTime=pts;
    }
#warning fix over-seg issue
    _VIDEOS   *vid=_segments.getRefVideo(ref);
    vidHeader *demuxer=vid->_aviheader;
    uint64_t  lastPts=demuxer->getTime(0);
    uint32_t  nb=demuxer->getVideoStreamHeader()->dwLength;

    if(lastPts>pts) return 0;

	for(int i=1;i<nb-2;i++)
    {
        uint64_t cur,next;
        cur=lastPts;
        next=demuxer->getTime(i+1);
        if(next==ADM_NO_PTS) next=cur+vid->timeIncrementInUs;
        if(pts>=cur && pts<next) return i-1;
        lastPts=next;
    }
    return nb-1;
}
/**
    \fn searchFrameBefore
    \brief Return the frame number with pts just before pts
*/
uint32_t ADM_Composer::searchFrameAt(uint64_t pts)
{
uint64_t refTime;
uint32_t ref;
    if(false==_segments.getRefFromTime(pts,&ref,&refTime))
    {
        ADM_warning(" Failed for pts %"LLU"\n",pts);
        ref=0;
        refTime=pts;
    }
#warning fix over-seg issue
    _VIDEOS   *vid=_segments.getRefVideo(ref);
    vidHeader *demuxer=vid->_aviheader;
    
    uint32_t  nb=demuxer->getVideoStreamHeader()->dwLength;
;

	for(int i=0;i<nb;i++)
    {
        uint64_t thisPts,thisDts;
        demuxer->getPtsDts(i,&thisPts,&thisDts);
        if(thisPts==ADM_NO_PTS) continue;
        if(refTime==thisPts)
        {
            ADM_info("Found frame %"LU" for time %"LLU" ms\n",i,thisPts/1000);
            return i;
        }
    }
    ADM_warning("Cannot find frame at %"LLU"\n",pts);
    return 0;
}
/**
    \fn getImageFromCacheForFrameBefore
    \brief Search the cache for the image with PTS just before the input PTS
*/
bool    ADM_Composer::getImageFromCacheForFrameBefore(uint64_t pts,ADMImage *out)
{
    int ref=0;
    EditorCache   *cache;
	_VIDEOS *vid=_segments.getRefVideo(0);
	cache=vid->_videoCache;
	ADM_assert(cache);
        ADMImage *r=cache->findLastBefore(pts);
        if(!r) return false;
        out->duplicateFull(r);
        return true;

}
/**
    \fn getPtsDts
    \brief Return PTS & DTS for a given frame (in bitstream order)
*/
bool        ADM_Composer::getPtsDts(uint32_t frame,uint64_t *pts,uint64_t *dts)
{
uint32_t ref,refOffset;
    if(_segments.getRefFromFrame(frame,&ref,&refOffset)==false)
    {
        ADM_warning("Cannot get ref video for frame %"LD"\n",frame);
        return false;
    }
 
     _VIDEOS   *vid=_segments.getRefVideo(ref);
    vidHeader *demuxer=vid->_aviheader;
    return demuxer->getPtsDts(frame,pts,dts); // FIXME : Rescale frame number
}
/**
    \fn getDurationInUs
    \brief Return total duration of video in us
*/
 uint64_t        ADM_Composer::getDurationInUs(void) 
{
    return _segments.getTotalDuration();
}
//EOF