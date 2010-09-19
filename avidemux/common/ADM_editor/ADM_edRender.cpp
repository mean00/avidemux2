/***************************************************************************
    \file  ADM_edRender.cpp  
    \brief handle decoding by masking the editor segments (appended video, cut, etc..)
    \author mean (c) 2002/2009 fixounet@free.fr

    

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
#include "ADM_cpp.h"
#include "ADM_default.h"
#include "ADM_editor/ADM_edit.hxx"

#if 0
#define aprintf printf
#define SET_CURRENT_PTS(x) {printf("Old PTS: %"LLU" ms, new PTS %"LLU" ms\n",_currentPts,x);_currentPts=x;}
#else
#define aprintf(...) {}// printf
#define SET_CURRENT_PTS(x) {_currentPts=x;}
#endif

#include "ADM_pp.h"

/**
    \fn recalibrateSigned
    \brief Same as below, but can return negative number
*/
void ADM_Composer::recalibrateSigned(int64_t *time,_SEGMENT *seg)
{
int64_t t=(int64_t)*time;
        if(*time==ADM_NO_PTS) return;

        t-=seg->_refStartTimeUs;
        t+=seg->_startTimeUs;
        if(t==ADM_NO_PTS)
                t=ADM_NO_PTS-1;
        *time=(int64_t )t;
}
/**
    \fn recalibrate
    \brief Convert time given in time from absolute ref video to linear time
*/
void ADM_Composer::recalibrate(uint64_t *time,_SEGMENT *seg)
{
int64_t t=(int64_t)*time;
        if(*time==ADM_NO_PTS) return;

        t-=seg->_refStartTimeUs;
        if(t<0)
        {
            ADM_warning("Segment time is negative time : %"LLU" ms, refStartTime:%"LLU" ms!\n",*time/1000,seg->_refStartTimeUs/1000);
        }
        t+=seg->_startTimeUs;
        if(t<0)
        {
            ADM_error("Absolute time is negative time : %"LLD" ms, _startTime:%"LLU" ms!\n",t/1000,seg->_startTimeUs/1000);
            t=0;
        }
        *time=(uint64_t )t;
}
/**
    \fn updateImageTiming
*/
bool ADM_Composer::updateImageTiming(_SEGMENT *seg,ADMImage *image)
{
    recalibrate(&(image->Pts),seg);
//    recalibrate(&(image->Dts),seg);
    return true;
}
/**
    \fn GoToIntraTime_noDecoding
    \brief Go to an intra at time time (exact) but do not decode frames
    \return true on success, false on error
*/
bool        ADM_Composer::GoToIntraTime_noDecoding(uint64_t time,uint32_t *toframe)
{
    uint32_t s;
    uint64_t segTime;
    // Search the seg ..;
    if(false==_segments.convertLinearTimeToSeg(time,&s,&segTime))
    {
        ADM_warning("GoToIntraTime failed!\n");
        return false;
    }
    _SEGMENT *seg=_segments.getSegment(s);
    ADM_assert(seg);
    _VIDEOS *ref=_segments.getRefVideo(seg->_reference);
    ADM_assert(ref);
    //

    uint64_t refTime=seg->_refStartTimeUs+segTime;

    uint32_t frame=_segments.intraTimeToFrame(seg->_reference,refTime);    
    if(s!=_currentSegment)
    {
        if(false==switchToSegment(s))
        {
            ADM_warning("Cannot go to segment %"LU"\n",s);
            return false;
        }
    }
    if(toframe) *toframe=frame;
    ref->lastSentFrame=frame; // For copy
    // Initialize _nextFrameDts, in fact next DTS
    uint64_t pts,dts;
    ref->_aviheader->getPtsDts(frame,&pts,&dts);
    if(dts==ADM_NO_PTS)
    {
        if(pts==ADM_NO_PTS) 
        {
            ADM_warning("No PTS nor DTS, cannot set start DTS");
            return false; // Fixme we can still guess DTS
        }
        // convert to linear time
        _segments.dtsFromPts(seg->_reference,pts,&dts);
    }
        time=(int64_t)dts;
        time-=seg->_refStartTimeUs;
        time+=seg->_startTimeUs;
        dts=time;
    
    _nextFrameDts=dts;
    seg->_dropBframes=1;
    return true;
}
/**
    \fn GoToIntraTime
    \brief Go to an intra at time time (exact)
    \return true on success, false on error
*/
bool        ADM_Composer::goToIntraTimeVideo(uint64_t time)
{
    uint32_t frame;
    if(false==GoToIntraTime_noDecoding(time,&frame))
    {
        ADM_warning("Seek failed.\n");
        return false;
    }
    _SEGMENT *seg=_segments.getSegment(_currentSegment);
    // Ok, we have switched to a new segment
    // Flush the cache
    _VIDEOS *vid= _segments.getRefVideo(seg->_reference);
    if(false== DecodePictureUpToIntra(seg->_reference,frame))
    {
        return false;
    }
    // Get the last decoded PTS and it is our current PTS
    uint64_t newPts=vid->lastDecodedPts+seg->_startTimeUs;
    newPts-=seg->_refStartTimeUs;
    SET_CURRENT_PTS(newPts);
#if 0
    ADM_info("decodec DTS=%"LLU" ms\n",vid->lastDecodedPts/1000);
    ADM_info("startTime DTS=%"LLU" ms\n",seg->_startTimeUs/1000);
    ADM_info("refstart DTS=%"LLU" ms\n",seg->_refStartTimeUs/1000);
    ADM_info("Current DTS=%"LLU" ms\n",_currentPts/1000);
#endif
    return true;
}
/**
    \fn goToTimeVideo
    \brief Seek video to the given time. Must be an exact time.
*/
bool  ADM_Composer::goToTimeVideo(uint64_t startTime)
{
uint64_t segTime;
uint32_t seg;
    if(false==_segments.convertLinearTimeToSeg(startTime,&seg,&segTime))
    {
        ADM_warning("Cannot find segment for time %"LLU" ms\n",startTime/1000);
        return false;
    }
    
    // Try to seek...
    _SEGMENT *s=_segments.getSegment(seg);
    _VIDEOS *v=_segments.getRefVideo(s->_reference);
    if(!s->_reference && !segTime && s->_refStartTimeUs<v->firstFramePts)
    {
        segTime=v->firstFramePts;
        ADM_warning("Fixating start time to %"LLU" ms\n",segTime/1000);
    }
    uint64_t to=segTime+s->_refStartTimeUs;
    if(false==seektoTime(s->_reference,to))
    {
            ADM_warning("Cannot seek to beginning of segment %"LU" at  %"LLU" ms\n",s,to/1000);
            return false;
    }
    _currentSegment=seg;
    int64_t newTime=(int64_t)v->lastDecodedPts+(int64_t)s->_startTimeUs-(int64_t)s->_refStartTimeUs;
    ADM_info("Seek done, in reference, gone to %"LLU" with segment start at %"LLU"\n",v->lastDecodedPts,s->_refStartTimeUs);
    SET_CURRENT_PTS(newTime);
    return true;

}

/**
    \fn NextPicture
    \brief decode & returns the next picture
*/
bool        ADM_Composer::nextPicture(ADMImage *image,bool dontcross)
{
uint64_t pts;
uint64_t tail;
    
        // Decode image...
        _SEGMENT *seg=_segments.getSegment(_currentSegment);
        // Search it in the cache...
        _VIDEOS *vid=_segments.getRefVideo(seg->_reference);
        
        uint64_t refPts;
        _segments.LinearToRefTime(_currentSegment,_currentPts,&refPts);
        ADMImage *cached=vid->_videoCache->getAfter(refPts);
        if(cached)
        {
            uint64_t delta=cached->Pts-seg->_refStartTimeUs;
            if(delta<seg->_durationUs)
            {
                // Got it
                image->duplicate(cached);
            }else
                goto np_nextSeg;
        }else 
        { // Not in cache, decode next one
            if(false== nextPictureInternal(seg->_reference,image))
            {
                goto np_nextSeg;
            }
        }
        // Got it, update timing
        updateImageTiming(seg,image);
        // no we have our image, let's check it is within this segment range..
        pts=image->Pts;
        tail=seg->_startTimeUs+seg->_durationUs;
        if(pts>=tail)
        {
                ADM_info("Got an image (%"LU" ms, but is out of this segment (%"LU"+%"LU"=%"LU" ms)\n",
                                                                    pts,seg->_startTimeUs,seg->_durationUs,tail);
                _segments.dump();
                goto np_nextSeg;
        }
        
        SET_CURRENT_PTS(pts);
        //ADM_info("Current PTS:%"LLU"\n",_currentPts);
        return true;

// Try to get an image for the following segment....
np_nextSeg:
        if(true==dontcross)
        {
            ADM_warning("Not allowed to cross segment\n");
            return false;
        }
        if(_currentSegment+1<_segments.getNbSegments())
        {
            if(switchToNextSegment()==false)
            {
                ADM_warning("Cannot get next picture. cannot go to next segment also !\n");
                return false;
            }
            ADM_info("Switched to next segment\n");
            seg=_segments.getSegment(_currentSegment);
            samePictureInternal(seg->_reference,image);
            updateImageTiming(seg,image);
            SET_CURRENT_PTS(pts);
            return true;
        } 
        ADM_warning("Cannot get next picture. Last segment\n");
        return false;
}
/**
    \fn decodeTillPicture
    \brief that one is a bit different compare to goToTime as we
                dont want to decode the targetPts if it is a keyframe
                but have the frame before
*/
bool ADM_Composer::decodeTillPictureAtPts(uint64_t targetPts,ADMImage *image)
{
 // Go to the previous keyframe and decode forward...
                uint32_t thisSeg=_currentSegment;
                _SEGMENT *seg=_segments.getSegment(_currentSegment);
                int ref=seg->_reference;

                uint64_t refTime;
                if(false==_segments.LinearToRefTime(_currentSegment,targetPts-1,&refTime))
                {
                    ADM_warning("Cannot find ref time\n");
                    return false;
                }
                uint64_t previousKf;
                if(false==searchPreviousKeyFrameInRef(ref,refTime,&previousKf))
                {
                    ADM_warning("Cannot find previous keyframe in ref %d, time=%"LLU" \n",ref,refTime);
                    return false;
                }
                // go to it...
                if(false==seektoTime(ref,previousKf,false))
                {
                    ADM_warning("Cannot seek to time=%"LLU" \n",previousKf);
                    return false;            
                }
                // Now forward till we reach out frame
                while(1)
                {
                    if(false==nextPicture(image,true))
                    {
                            ADM_warning("Error in decoding forward");
                            return false;
                    }
                    if(image->Pts>=targetPts)
                            break;
                    if(thisSeg!=_currentSegment)
                            break;
                }
                if(image->Pts!=targetPts)
                {
                    ADM_error("Could not retrieve our own frame at PTS=%"LLU" ms\n",targetPts/1000);
                    return false;
                }
                return true;
}
/**
    \fn NextPicture
    \brief decode & returns the next picture
*/
bool        ADM_Composer::previousPicture(ADMImage *image)
{
uint64_t targetPts=_currentPts;    
        // Decode image...
        _SEGMENT *seg=_segments.getSegment(_currentSegment);
        // Search it in the cache...
        _VIDEOS *vid=_segments.getRefVideo(seg->_reference);
        
        uint64_t refPts;

        _segments.LinearToRefTime(_currentSegment,_currentPts,&refPts);
        ADMImage *cached=vid->_videoCache->getBefore(refPts);
       
        if(cached)
        {
            if(cached->Pts>=seg->_refStartTimeUs) // It might be in the cache but belonging to the previous seg
            {
                // Got it
                image->duplicate(cached);
                updateImageTiming(seg,image);
                SET_CURRENT_PTS(image->Pts);
                return true;
            }
        }
        ADM_info("while looking for frame %"LLU"\n",_currentPts);
        vid->_videoCache->dump();
        // The previous is not available
        // either it is in the same segment but we have decoded later in that segment
        // , or it is in the previous segment
        // Let's check...
        if(!_currentPts) return false;
        uint64_t segTime;
        uint32_t segNo;
            if(false==_segments.convertLinearTimeToSeg(_currentPts-1,&segNo,&segTime))
            {
                  ADM_error("Cannot convert time in samePicture\n");         
                  return false;
            }
        // Two possiblities
        // 1- we are still in the same segment, in that case, we have to go back
        // and decode forward 
        // 2- We are at a jum, i.e. what we want is the last image from the previous segment
        seg=_segments.getSegment(segNo);
        if(segNo==_currentSegment) // Still in the same segment..
        {
                if(false==decodeTillPictureAtPts(targetPts,image))
                {
                    ADM_error("Cannot decode till our current pts\n");
                    return false;
                }
                cached=vid->_videoCache->getBefore(refPts);
                if(cached)
                {
                    if(cached->Pts>=seg->_refStartTimeUs)
                    {
                        // Got it
                        image->duplicate(cached);
                        updateImageTiming(seg,image);
                        SET_CURRENT_PTS(image->Pts);
                        return true;
                    }
                    ADM_warning("The image found is before refStartTime ???\n");
                }
                ADM_error("Find our frame and its predecessor (%"LLU"), but it is out of range\n",refPts);
                vid->_videoCache->dump();
                return false;
        }
        ADM_assert(segNo+1==_currentSegment);
        // it is basically the same as above except the exit condition is that the frame is out of reach
        // Either because we reached end of segment or end of source for that segment
        ADM_info("Searching across segment....\n");
        if(false==switchToSegment(segNo))
        {
            ADM_error("Cannot switch to previous segment to get previous frame\n");
            return false;
        }
        
        seg=_segments.getSegment(_currentSegment);
        vid=_segments.getRefVideo(seg->_reference);
        

        decodeTillPictureAtPts(targetPts,image);
        _currentSegment=segNo;
        // We may have overshot...
        uint64_t last=vid->lastDecodedPts;
        _segments.LinearToRefTime(_currentSegment,targetPts,&refPts);
        ADMImage *candidate=vid->_videoCache->getLast();
        if(!candidate)
        {
            ADM_error("No frame in cache !\n");
            return false;
        }
        while(1)
        {
            if(candidate->Pts<refPts) // got it!
            {
                break;
            }
            // Try to go before...
            ADMImage *img=vid->_videoCache->getBefore(candidate->Pts);
            if(!img) break;
            candidate=img;
        }
        image->duplicate(candidate);
        updateImageTiming(seg,image);
        SET_CURRENT_PTS(image->Pts);
        return true;
        
        
}
/**
    \fn samePicture
    \brief returns the last already decoded picture
*/
bool        ADM_Composer::samePicture(ADMImage *image)
{
      _SEGMENT *seg=_segments.getSegment(_currentSegment);
      _VIDEOS  *ref=_segments.getRefVideo(seg->_reference);

        // Do Pts->ref PTs
uint64_t refPts;
uint64_t segTime;
uint32_t segNo;
    if(false==_segments.convertLinearTimeToSeg(_currentPts,&segNo,&segTime))
    {
          ADM_error("Cannot convert time in samePicture\n");         
          return false;
    }
      ADM_assert(_currentSegment==segNo);
      refPts=segTime+seg->_refStartTimeUs;
      ADMImage *last=ref->_videoCache->getByPts(refPts);
      if(last)
      {
            image->duplicate(last);
            updateImageTiming(seg,image);
            return true;
      }
      ADM_error("Cannot find same image in cache\n"); 
      ADM_info("Looking for PTS=%"LLU" ms\n",refPts/1000);
      ref->_videoCache->dump();
      return false;
}


/**
        \fn dupe
*/
uint8_t ADM_Composer::dupe(ADMImage *src,ADMImage *dst,_VIDEOS *vid)
{
                if(src->_colorspace!=ADM_COLOR_YV12)
                {
                        // We need to do some colorspace conversion
                        // Is there already one ?
                        if(!vid->color)
                        {
                              //  vid->color=new COL_Generic2YV12(src->_width,src->_height,src->_colorspace);
                              vid->color=new ADMColorScalerSimple(src->_width,src->_height,
                                                                 src->_colorspace,ADM_COLOR_YV12);
                        }
                        // Since it is not YV12 it MUST be a ref
                        ADM_assert(src->isRef());
                        
                        uint32_t srcStrides[3];
                        uint8_t  *srcPlanes[3];
                        uint32_t dstStrides[3];
                        uint8_t  *dstPlanes[3];

                        src->GetPitches(srcStrides);
                        src->GetReadPlanes(srcPlanes);
                        dst->GetPitches(dstStrides);
                        dst->GetWritePlanes(dstPlanes);

                        vid->color->convertPlanes(srcStrides,dstStrides,srcPlanes,dstPlanes);
                        return 1;
                }
                // nothing to do
//                if(_pp.swapuv)
#warning handle swap uv
                dst->duplicate(src);
                return 1;
}
/**
    \fn setPostProc
*/
uint8_t ADM_Composer::setPostProc( uint32_t type, uint32_t strength, uint32_t swapuv)
{
	if(!_segments.getNbRefVideos()) return 0;
    if(!_pp) return false;
	_pp->postProcType=type;
	_pp->postProcStrength=strength;
    _pp->swapuv=swapuv;
	_pp->update(); // DeletePostproc/ini missing ?
	return 1;
}
/**
    \fn getPostProc
*/

uint8_t ADM_Composer::getPostProc( uint32_t *type, uint32_t *strength, uint32_t *swapuv)
{
	if(!_segments.getNbRefVideos()) return 0;
    if(!_pp) return false;
	*type=_pp->postProcType;
	*strength=_pp->postProcStrength;
	*swapuv=_pp->swapuv;
	return 1;
}
/**
    \fn switchToNextSegment
    \brief Switch to the next segment

*/  
bool        ADM_Composer::switchToNextSegment(bool dontdecode)
{
    if(_currentSegment==_segments.getNbSegments()-1)
    {
        ADM_warning("This is the last segment (%"LU")\n",_currentSegment);
        return false;
    }
    if(true==switchToSegment(_currentSegment+1,dontdecode)) return true;
    return false;
}
/**
    \fn switchToSegment
    \brief Switch to the segment given as argument

*/
bool        ADM_Composer::switchToSegment(uint32_t s,bool dontdecode)
{
    if(s+1>_segments.getNbSegments())
    {
        ADM_warning("Cannot switch to segment:%"LU"\n",s);
        return false;
    }
    _SEGMENT *seg=_segments.getSegment(s);
    ADM_assert(seg);
    ADM_info("Trying to switch to seg %"LU" with startTime in reference pic= %"LU" ms\n",s,seg->_refStartTimeUs/1000);
        // If the refStartTime is 0, it is the first image
        // But the fist image might not be = 0
      _VIDEOS *vid=_segments.getRefVideo(seg->_reference);
      vidHeader 	*demuxer=vid->_aviheader;  
      ADM_assert(vid);
      uint64_t from=seg->_refStartTimeUs;
      uint64_t pts,dts;

      if(!from) from=vid->firstFramePts;

    
    if(false==seektoTime(seg->_reference,from,dontdecode))
    {
            ADM_warning("Cannot seek to beginning of segment %"LU" at  %"LLU" ms\n",s,from/1000);
            return false;
    }
    _currentSegment=s;
    ADM_info("Switched ok to segment %"LU" (dontdecode=%d)\n",s,dontdecode);
    return true;
}
/**
    \fn rewind
    \brief
*/
bool ADM_Composer::rewind(void)
{
        ADM_info("Rewinding\n");
        if(switchToSegment(0)==false) return false;
        _SEGMENT *seg=_segments.getSegment(0);
        _VIDEOS *vid=_segments.getRefVideo(seg->_reference);
        uint64_t pts=vid->lastDecodedPts;
        pts=pts+seg->_startTimeUs;
        pts-=seg->_refStartTimeUs;
        SET_CURRENT_PTS(pts);
        return true;
}
/**
    \fn addSegment
    \brief add a segment. The startTime will be computed later.
*/
bool    ADM_Composer::addSegment(uint32_t ref, uint64_t startRef, uint64_t duration)
{
    ADM_assert(ref<_segments.getNbRefVideos());
    _SEGMENT seg;
    memset(&seg,0,sizeof(seg));
    seg._durationUs=duration;
    seg._reference=ref;
    seg._refStartTimeUs=startRef;
    return _segments.addSegment(&seg);
}
/**
    \fn clearSegment
    \brief empty the segment list
*/  
bool   ADM_Composer::clearSegment(void)
{
    return _segments.deleteSegments();
}

//EOF
