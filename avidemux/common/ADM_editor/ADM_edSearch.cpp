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
    // Special case : The very first frame
    if(*frameTime<=1)
      {
          _VIDEOS *vid=_segments.getRefVideo(0);
          uint64_t pts,dts;
          vid->_aviheader->getPtsDts(0,&pts,&dts);
          if(pts!=ADM_NO_PTS)
            {
              if(pts>0)
                {
                    ADM_warning("This video does not start at 0 but at %"LLU" ms, compensating\n",pts/1000);
                    _segments.convertLinearTimeToSeg(  *frameTime+pts, &seg, &segTime);
                }
            }
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
            ADM_warning(" No previous previous keyfr for frameTime %"LLU"\n",*frameTime);
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
            ADM_info("Found nextkeyframe %"LLU" at frame %"LU"\n",pts/1000,i);
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
    ADM_warning("Cannot find keyframe with PTS less than %"LU"ms\n",refTime/1000);
    return false;
}

/**
    \fn getDurationInUs
    \brief Return total duration of video in us
*/
 uint64_t        ADM_Composer::getDurationInUs(void) 
{
    return _segments.getTotalDuration();
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
        ADM_warning(" Cannot find seg for time %"LLD"\n",*time);
        return false;
    }  
    _SEGMENT *s=_segments.getSegment(seg);
    //
    // Search the frame with correct PTS
    uint64_t pts=segTime+s->_refStartTimeUs;
    uint64_t dts;
    if(false==_segments.ptsFromDts(s->_reference,pts,&dts))
    {
        ADM_warning("Cannot get DTS from PTS=%"LLU"ms\n",pts/1000);
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
