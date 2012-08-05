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
#include "ADM_cpp.h"
using std::string;
#include "ADM_default.h"
#include "fourcc.h"
#include "ADM_edit.hxx"
#include "ADM_vidMisc.h"
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
        ADM_warning(" Cannot find seg for time %"PRId64"\n",*frameTime);
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
        if(seg==_segments.getNbSegments()-1)
        {
            ADM_warning("Last segment\n");
            return false;
        }
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
        ADM_warning(" Cannot find seg for time %"PRId64"\n",*frameTime);
        return false;
    }   
    // Special case : The very first frame FIXME
    // Only applies if the first segment as a 0 ref time, i.e. beginning of video..
    if(*frameTime<=1 && seg==0)
      {
          _VIDEOS *vid=_segments.getRefVideo(0);
          _SEGMENT *s=_segments.getSegment(seg);
          if(!s->_refStartTimeUs)
          {
              uint64_t pts=vid->firstFramePts;
              //
              *frameTime+=pts;
              ADM_warning("This video does not start at 0 but at %"PRIu64" ms, compensating\n",pts/1000);
              _segments.convertLinearTimeToSeg(  *frameTime, &seg, &segTime);
           }
      }
    // 
again:
    _SEGMENT *s=_segments.getSegment(seg);
    int64_t delta=*frameTime-s->_startTimeUs; // Delta compared to the beginning of this seg
    
    delta+=s->_refStartTimeUs;
    if(delta<0)
    {
        ADM_error("Time is negative\n");
        return false;
    }
    uint32_t ref=s->_reference;
    // 2- Now search the previous keyframe in the ref image...
    // The time in reference = relTime+segmentStartTime
    refTime=delta;
    
    r=searchPreviousKeyFrameInRef(ref,refTime,&nkTime);

    // 3- if it does not belong to the same seg  ....
    if(r==false || nkTime < (s->_refStartTimeUs))
    {
        if(!seg)
        {
            ADM_warning(" No previous previous keyfr for frameTime %"PRIu64" in ref %"PRIu32" seg:%"PRIu32" nkTime %"PRIu64" refTime:%"PRIu64" ms startTime=%"PRIu64" r=%d\n",
                            *frameTime,ref,seg,nkTime/1000,refTime/1000,s->_refStartTimeUs/1000,r);
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
    \fn getPtsDtsDelta
*/

bool			ADM_Composer::getPtsDtsDelta(uint64_t *frameTime)
{
uint64_t refTime,nkTime,segTime;
int lastSeg=_segments.getNbSegments();
uint32_t seg;
bool r;
    // 1- Convert frameTime to segments
    if(false== _segments.convertLinearTimeToSeg(  *frameTime, &seg, &segTime))
    {
        ADM_warning(" Cannot find seg for time %"PRId64"\n",*frameTime);
        return false;
    }   
    // 
    _SEGMENT *s=_segments.getSegment(seg);
    int64_t delta=*frameTime-s->_startTimeUs; // Delta compared to the beginning of this seg
    
    delta+=s->_refStartTimeUs;
    if(delta<0)
    {
        ADM_error("Time is negative\n");
        return false;
    }
    // Delta is now the absolute PTS  time in reference video
    uint32_t ref=s->_reference;
    refTime=delta;
    uint64_t dts;
    if(false==_segments.dtsFromPts(ref,refTime,&dts))
    {
        ADM_error("Cannot get dtsFromDts for time %"PRIu64"\n",refTime);
        *frameTime=0;
        return false;
    }
    // Ok we have PTS and DTS, returns difference
    *frameTime=refTime-dts;
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
            uint32_t hh,mm,ss,ms,ms2;
            ms=pts/1000;
            ms2time(ms,&hh,&mm,&ss,&ms2);
            ADM_info("Found nextkeyframe %"PRIu64" %u:%u:%u at frame %"PRIu32"\n",ms,hh,mm,ss,i);
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
    ADM_warning("Cannot find keyframe with PTS less than %"PRIu32"ms\n",refTime/1000);
    return false;
}

/**
    \fn getDtsFromPts
    \brief Estimate DTS from PTS

*/
bool        ADM_Composer::getDtsFromPts(uint64_t *time)
{
uint64_t refTime,nkTime,segTime;
int lastSeg=_segments.getNbSegments();
uint32_t seg;
    // 1- Convert frameTime to segments
    if(false== _segments.convertLinearTimeToSeg(  *time, &seg, &segTime))
    {
        ADM_warning(" Cannot find seg for time %"PRId64"\n",*time);
        return false;
    }  
    _SEGMENT *s=_segments.getSegment(seg);
    //
    // Search the frame with correct PTS
    uint64_t pts=segTime+s->_refStartTimeUs;
    uint64_t dts;
    if(false==_segments.dtsFromPts(s->_reference,pts,&dts))
    {
        ADM_warning("Cannot get DTS from PTS=%"PRIu64"ms\n",pts/1000);
        return false;
    }
    dts=dts+s->_startTimeUs;
    if(dts<s->_refStartTimeUs)
    {
        ADM_warning("Warning DTS time is negative\n");
        dts=0;
    }else
        dts=dts-s->_refStartTimeUs;
    *time=dts;
    return true;
}
//EOF
