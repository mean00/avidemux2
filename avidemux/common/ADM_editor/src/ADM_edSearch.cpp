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

#define pivotPrintf(...) {}

/**
    \fn getNKFramePTS
*/
bool	ADM_Composer::getNKFramePTS(uint64_t *frameTime)
{
uint64_t refTime,nkTime,segTime;
int lastSeg=_segments.getNbSegments();
uint32_t seg;
bool switchedSeg=false;
bool r;
    // 1- Convert frameTime to segments
    if(false== _segments.convertLinearTimeToSeg(  *frameTime, &seg, &segTime))
    {
        ADM_warning(" Cannot find seg for time %" PRId64"\n",*frameTime);
        return false;
    }   
    // 
again:
    _SEGMENT *s=_segments.getSegment(seg);
    uint32_t ref=s->_reference;
    // 2- Now search the previous keyframe in the ref image...
    // The time in reference = relTime+segmentStartTime
    refTime=s->_refStartTimeUs+segTime; // Absolute time in the reference image
    
    if(switchedSeg && refTime>0) refTime--; // The next segment may start with a keyframe

    r=searchNextKeyFrameInRef(ref,refTime,&nkTime);

    // 3- if it does not belong to the same seg  ....
    if(r==false || nkTime >= (s->_refStartTimeUs+s->_durationUs))
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
        switchedSeg=true;
        segTime=0;
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
        ADM_warning(" Cannot find seg for time %" PRId64"\n",*frameTime);
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
              ADM_warning("This video does not start at 0 but at %" PRIu64" ms, compensating\n",pts/1000);
              _segments.convertLinearTimeToSeg(  *frameTime, &seg, &segTime);
           }
      }
    // 
again:
    _SEGMENT *s=_segments.getSegment(seg);
    int64_t delta=*frameTime-s->_startTimeUs; // Delta compared to the beginning of this seg
    if(delta >= s->_durationUs) delta=s->_durationUs-1;
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
            ADM_warning(" No previous previous keyfr for frameTime %" PRIu64" in ref %" PRIu32" seg:%" PRIu32" nkTime %" PRIu64" refTime:%" PRIu64" ms startTime=%" PRIu64" r=%d\n",
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

bool			ADM_Composer::getPtsDtsDelta(uint64_t frameTime, uint64_t *outDelta)
{
uint64_t refTime,nkTime,segTime;
int lastSeg=_segments.getNbSegments();
uint32_t seg;
bool r;
    // 1- Convert frameTime to segments
    if(false== _segments.convertLinearTimeToSeg(  frameTime, &seg, &segTime))
    {
        ADM_warning(" Cannot find seg for time %" PRId64"\n",frameTime);
        return false;
    }   
    // 
    _SEGMENT *s=_segments.getSegment(seg);
    int64_t delta=(int64_t)frameTime-(int64_t)s->_startTimeUs; // Delta compared to the beginning of this seg
    
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
        ADM_error("Cannot get dtsFromDts for time %" PRIu64"\n",refTime);
        *outDelta=0;
        return false;
    }
    // Ok we have PTS and DTS, returns difference
    *outDelta=refTime-dts;
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
            ADM_info("Found nextkeyframe %" PRIu32" %u:%u:%u at frame %" PRIu32"\n",ms,hh,mm,ss,i);
            *nkTime=pts;
            return true;
        }
    }
    return false;
}
/** 
 * \fn getFrameNumberBeforePts
 * \brief Search the framenumber that has a valid PTS = what's given or  before
 *         it can be equal to refTime
 * @param v
 * @param refTime
 * @param frameNumber
 * @return 
 */
bool ADM_Composer::getFrameNumFromPtsOrBefore(_VIDEOS *v,uint64_t refTime,int &frameNumber)
{
    uint32_t nbFrame = v->_nb_video_frames;
    uint64_t pts, dts;
    uint32_t curFrame=nbFrame >> 1;
    uint32_t splitMoval = curFrame >> 1;
    pivotPrintf("Looking for frame with a pts   %s\n",ADM_us2plain(refTime));
    // Try to find the frame that as the timestamp close enough to refTime, while being smaller
    do 
    {
        // seems like the pts determination, not always works -> retry if necessary
        bool worked;
        do {
                pts=ADM_NO_PTS;
                worked = v->_aviheader->getPtsDts(curFrame,&pts,&dts);
                if (worked && pts != ADM_NO_PTS) 
                    break; // found                
                curFrame--;
                if (!curFrame)
                {
                        ADM_warning("The whole segment is corrupted. Abort the search");
                        return false;
                }
        } while(true);
        pivotPrintf("SplitMoval=%d\n",splitMoval);
        if(pts==refTime)
            break;
        if (!splitMoval) 
            break;        
        pivotPrintf("Pivot time is %d, %s\n",curFrame,ADM_us2plain(pts));
        if (pts >= refTime)
                curFrame -= splitMoval;
        else
                curFrame += splitMoval;
        splitMoval >>= 1;
        pivotPrintf("Split=%d\n",splitMoval);        
    } while(refTime != pts);

    pivotPrintf("-Matching frame is %d, time is  %s\n",curFrame,ADM_us2plain(pts));
    if (pts > refTime && curFrame)
            curFrame--;
    frameNumber=curFrame;
    return true;
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
    // Search for the current frame with quick search
    _VIDEOS *v = _segments.getRefVideo(ref);
    uint32_t nbFrame = v->_nb_video_frames;
    uint64_t pts, dts;
    int curFrame;

    // look up the frame we are starting from...
    if(!getFrameNumFromPtsOrBefore(v,refTime,curFrame))
    {
        ADM_warning("Cannot locate the frame we are talking about at PTS=%s\n",ADM_us2plain(refTime));
        return false;
    }
    // rewind until we find a keyframe
    for (int i=curFrame; i>=0; i--) 
    {
    uint64_t p;
    uint32_t flags;
            v->_aviheader->getFlags(i,&flags);
            if(!(flags & AVI_KEY_FRAME)) 
                continue;
            if(!v->_aviheader->getPtsDts(i,&pts,&dts))
                continue;
            if(pts == ADM_NO_PTS)
                continue;
            pivotPrintf("-- Previous Kf ! This one is %s\n",ADM_us2plain(pts));
            pivotPrintf("-- Ref is %s\n",ADM_us2plain(refTime));
            pivotPrintf("-- DTS %s\n",ADM_us2plain(dts));
            if(pts>=refTime) 
                continue;
            *nkTime = pts; // ok found it
#if 0            
            for(int j=0;j<5;j++)
            {
                 if(!v->_aviheader->getPtsDts(i+j,&pts,&dts)) continue ;
                 printf("Neighbor [%d] = %s\n",j,ADM_us2plain(pts));
            }
#endif            
            return true;
    }
    ADM_warning("Cannot find keyframe with PTS less than %s\n",ADM_us2plain(refTime));
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
        ADM_warning(" Cannot find seg for time %" PRId64"\n",*time);
        return false;
    }  
    _SEGMENT *s=_segments.getSegment(seg);
    //
    // Search the frame with correct PTS
    uint64_t pts=segTime+s->_refStartTimeUs;
    uint64_t dts;
    if(false==_segments.dtsFromPts(s->_reference,pts,&dts))
    {
        ADM_warning("Cannot get DTS from PTS=%" PRIu64"ms\n",pts/1000);
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
/**
 * \fn getLastKeyFramePts
 * \brief returns the pts of the last accessible keyframe
 * @return 
 */
uint64_t    ADM_Composer::getLastKeyFramePts(void)
 {         
    int lastSeg=_segments.getNbSegments();
    int seg;
    uint64_t pts,dts;
    int lastRefFrameInSegment;
    if(!lastSeg) return ADM_NO_PTS;
    for(seg=lastSeg-1;seg>=0;seg--)
    {
          _SEGMENT *s=_segments.getSegment(seg);
          _VIDEOS  *v=_segments.getRefVideo(s->_reference);
          uint64_t endTimeInRef=(s->_refStartTimeUs)+(s->_durationUs);
          if(!getFrameNumFromPtsOrBefore(v,endTimeInRef,lastRefFrameInSegment))
          {
              ADM_warning("Cannot map the last frame in segment to reference at PTS=%s\n",ADM_us2plain(endTimeInRef));
              lastRefFrameInSegment=v->_nb_video_frames;
          }
          int nbFrame=lastRefFrameInSegment;
          if(!nbFrame) break;

          for(int frame=nbFrame-1;frame>=0;frame--)
          {
              uint32_t flags;
              pts=ADM_NO_PTS;    
              v->_aviheader->getFlags(frame,&flags);
              if(!(flags & AVI_KEY_FRAME)) continue;
              v->_aviheader->getPtsDts(frame,&pts,&dts);
              if(pts==ADM_NO_PTS) continue;          
              ADM_info("found last keyframe at %d, time in reference=%s\n",frame,ADM_us2plain(pts));
              break;
          }
          if(pts!=ADM_NO_PTS)
          {
              pts+=s->_startTimeUs-s->_refStartTimeUs;
              ADM_info("found last keyframe at %s\n",ADM_us2plain(pts));
              return pts;
          }
     }
    ADM_info("Cannot find lastKeyFrame with a valid PTS\n");
    return ADM_NO_PTS;
}
//EOF
