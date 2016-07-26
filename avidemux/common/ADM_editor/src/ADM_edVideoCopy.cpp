/***************************************************************************
    \file  ADM_edVideoCopy.cpp  
    \brief handle direct stream copy for video
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
#include "ADM_cpp.h"
#include <math.h>
#include "ADM_default.h"
#include "ADM_edit.hxx"
#include "ADM_vidMisc.h"
#if 0
#define aprintf printf
#else
#define aprintf(...) {}// printf
#endif

/**
    \fn checkCutsAreOnIntra
    \brief In copy mode, if the cuts are not on intra we will run into trouble :
            * We include skipped ref frames: we will have DTS going back error
            * We skip them, we have borked video at cut points due to missing ref framesz
    \return true if everything ok
*/
bool ADM_Composer::checkCutsAreOnIntra(void)
{
    bool fail=false;
    int nbSeg=_segments.getNbSegments();

    ADMCompressedImage img;
    uint8_t *buffer=new uint8_t[1920*1080*3];
    img.data=buffer;
    ADM_info("Checking cuts start on keyframe..\n");
    for(int i=0;i<nbSeg;i++)
    {
        _SEGMENT *seg=_segments.getSegment(i);
        _VIDEOS *vid=_segments.getRefVideo(seg->_reference);
        vidHeader *demuxer=vid->_aviheader;

        if(false==switchToSegment(i,true))
        {
            fail=true;
            break;
        }
        if(false==demuxer->getFrame (vid->lastSentFrame,&img))
        {
            ADM_info("Cannot get 1st frame of segment %d\n",i);
            fail=true;
            break;
        }
        if(!img.flags & AVI_KEY_FRAME)
        {
            ADM_warning("Segment %d does not start on a keyframe (%s)\n",i,ADM_us2plain(img.demuxerPts));
            fail=true;
            break;
        }
        // After a seg switch we are at the keyframe before or equal to where we want to go
        // if the dts do not match, it means we went back too much
        // When re-encoding, it's not a problem, it is when copying.
        ADM_info("seg:%d refDTS=%" PRIu64"\n",seg->_reference,seg->_refStartDts);
        ADM_info("seg:%d imgDTS=%" PRIu64"\n",seg->_reference,img.demuxerDts);
        if(!seg->_refStartDts && !seg->_reference)
        {
            ADM_info("Ignoring first seg (unreliable DTS)\n");
            
        }else
        if(img.demuxerDts!=ADM_NO_PTS && seg->_refStartDts!=ADM_NO_PTS && 
            img.demuxerDts!=seg->_refStartDts)
        {
            ADM_warning("Segment %d does not start on a known DTS (%s)\n",i,ADM_us2plain(img.demuxerPts));
            ADM_warning("expected (%s)\n",ADM_us2plain(seg->_refStartDts));
            fail=true;
            break;
        }
        ADM_info("Segment %d ok\n",i);
    }   
    delete [] buffer;
    buffer=NULL;
    if(fail) return false;
    return true;
}
/**
     \fn bFrameDroppable
*/
static bool bFrameDroppable(uint32_t fcc)
{
    if(isH264Compatible(fcc)) 
      return false;
    if(isH265Compatible(fcc)) 
      return false;    
    return true;
}
/**
 * 
 * @param time
 * @param delay
 * @return 
 */
bool        ADM_Composer::getNonClosedGopDelay(uint64_t time,uint32_t *delay)
{
  aviInfo    info;
  uint32_t   segNo;
  uint64_t   segTime;
    *delay=0;
    if(!_segments.convertLinearTimeToSeg(time,&segNo,&segTime))
      {
        ADM_warning("Cannot navigate to get nonclosedgop\n");
        return false;
      }
   _SEGMENT *seg=_segments.getSegment(segNo);
    ADM_assert(seg);
   _VIDEOS *vid=_segments.getRefVideo(seg->_reference);   
   uint64_t pts,dts;
   
    vid->_aviheader->getVideoInfo (&info);
    //
    if(bFrameDroppable(info.fcc))
    {
        return true; // no need to add extra delay
    }
    // Look ahead to see if we have some late bframe in the past
    int nb=vid->_aviheader->getMainHeader ()->dwTotalFrames;
    int found=-1;
    if(!segTime)
    {
      found=0;
       vid->_aviheader->getPtsDts(0,&pts,&dts);
    }
    else
        for(int i=0;i<nb;i++)
        {
            vid->_aviheader->getPtsDts (i,&pts,&dts);
            if(pts==segTime)
              {
                    found=i;
                    break;
              }
        }
    if(found==-1)
      {
        ADM_warning("Cannot find the frame\n");
        return false;
      }
    for(int i=found+1;i<found+16;i++)
    {
            uint32_t flags;
            vid->_aviheader->getFlags(i,&flags);
            if(!(flags & AVI_B_FRAME))
            {
                ADM_info("Not a bframe, stopping (%d)\n",i-found);
              break;
            }
            vid->_aviheader->getPtsDts(i,&pts,&dts);
            if(pts<segTime)
              {
                ADM_info("frame %d is early \n",i);
                uint32_t delta=segTime-pts;
                if(delta>*delay) *delay=delta;
              }else
                {
                    ADM_info("Pts delta = %d\n",(int)(pts-segTime));
                }
    }
    ADM_info("Found maximum non closed gop delay = %d\n",*delay);
    return true;
}

/**
        \fn getCompressedPicture
        \brief bypass decoder and directly get the source image

    The dropBframe is as follow :
            0 : Dont drop b frame
            1 : Follow the next bframes
            2 : Drop


*/
bool        ADM_Composer::getCompressedPicture(uint64_t videoDelay,ADMCompressedImage *img)
{
    uint64_t tail;
    //
    int64_t signedPts;
    int64_t signedDts;

againGet:
    static uint32_t fn;
    fn++;
    _SEGMENT *seg=_segments.getSegment(_currentSegment);
    ADM_assert(seg);
    _VIDEOS *vid=_segments.getRefVideo(seg->_reference);
    ADM_assert(vid);
    vidHeader *demuxer=vid->_aviheader;
    ADM_assert(demuxer);

    // Get next pic?
    if(false==demuxer->getFrame (vid->lastSentFrame,img))
    {
        ADM_info("Failed to get next frame for ref %" PRIu32"\n",seg->_reference);
        goto nextSeg;
    }
    aprintf("Got frame %d, PTS=%s  ",img->demuxerFrameNo,ADM_us2plain(img->demuxerPts));
    aprintf("DTS=%s  \n",ADM_us2plain(img->demuxerDts));
    vid->lastSentFrame++;
    //
    aviInfo    info;
    vid->_aviheader->getVideoInfo (&info);
    //
    if(bFrameDroppable(info.fcc))
    {
        if(img->flags & AVI_B_FRAME)
        {
            if(seg->_dropBframes==ADM_DROPPING) 
            {
                ADM_warning("%" PRIu32" Dropping bframes\n",fn);
                goto againGet;
            }
        }else
        { // not a bframe
            switch(seg->_dropBframes)
            {
                case ADM_NO_DROP                : break;
                case ADM_DROPPING               : seg->_dropBframes=ADM_NO_DROP;break;
                case ADM_DROP_MAYBE_AFER_SWITCH : seg->_dropBframes=ADM_DROPPING;break;
                default: break;
            }
        }
    }
    //aprintf("Bframe droppable=%d, lastSentFrame=%d\n",seg->_dropBframes,vid->lastSentFrame);
    // after a segment switch, we may have some frames from "the past"
    // if the cut point is not a keyframe, drop them
#if 1
    if( img->demuxerDts!=ADM_NO_PTS)
    {
        bool drop=false;
        if(_currentSegment  && img->demuxerDts<seg->_refStartDts)
        {
            ADM_info("Frame %d is in the past for this segment (%s)",vid->lastSentFrame,ADM_us2plain(img->demuxerDts));
            ADM_info("vs refstartdts %s\n",ADM_us2plain(seg->_refStartDts));
            ADM_info(" dts=%llu, ref=%llu\n",img->demuxerDts,seg->_refStartDts);
            drop=true;
        }
        // Seeking is not accurate when cutting on non intra
        // we might have some frames that are clearly too early , even in seg0
        if(img->demuxerPts!=ADM_NO_PTS && bFrameDroppable(info.fcc))
        {
            if(img->demuxerPts+seg->_startTimeUs<seg->_refStartTimeUs)
            {
                ADM_info("Frame %d is in the past for this segment -bis (%s)",vid->lastSentFrame,ADM_us2plain(img->demuxerPts));
                ADM_info("vs refstartdts %s\n",ADM_us2plain(seg->_refStartTimeUs));
                ADM_info(" pts=%llu, startTime=%llu, _refStartTimeUs=%llu\n",img->demuxerPts,seg->_startTimeUs,seg->_refStartTimeUs);
                drop=true;
            }
        }
        if(drop)
            goto againGet;
        
    }
#endif
    // Need to switch seg ?
    tail=seg->_refStartTimeUs+seg->_durationUs;
    // Guess DTS
    //
//**
   // ADM_info("Frame : Flags :%X, DTS:%"PRId64" PTS=%"PRId64" nextDts=%"PRId64" tail=%"PRId64"\n",img->flags,img->demuxerDts/1000,img->demuxerPts/1000,_nextFrameDts,tail);
    if(img->demuxerDts!= ADM_NO_PTS && img->demuxerDts>=tail) 
    {
        aprintf("DTS is too late, switching (%s)\n",ADM_us2plain(img->demuxerDts));
        goto nextSeg;
    }
    if(img->demuxerPts!= ADM_NO_PTS && img->demuxerPts>=tail) 
    {
        aprintf("PTS is too late, switching (%s)\n",ADM_us2plain(img->demuxerDts));
        goto nextSeg;
    }
    
    // Since we rely on PTS for seeking, frame 0 might be at PTS 0, in that case the matching dts would be <0
    // so the caller can delay everything but recalibrate will clamp the value
    // so we use correctedDts so that the value is ok
    if(img->demuxerDts==ADM_NO_PTS)
            signedDts=ADM_NO_PTS;
    else
    {
            signedDts=(int64_t)img->demuxerDts;
            recalibrateSigned(&(signedDts),seg);
    }

    if(img->demuxerPts==ADM_NO_PTS)
            signedPts=ADM_NO_PTS;
    else
    {
            signedPts=(int64_t)img->demuxerPts;
            recalibrateSigned(&(signedPts),seg);
    }
    aprintf("Signed Pts=%s ",ADM_us2plain(signedPts));
    aprintf("Signed Dts=%s \n",ADM_us2plain(signedDts));
    // From here we are in linear time, guess DTS if missing...
    if(signedDts==ADM_NO_PTS)
    {
	// border case due to rounding we can have pts slighly above dts
	if(signedPts!=ADM_NO_PTS && _nextFrameDts!=ADM_NO_PTS)
	{
        signedDts=_nextFrameDts;
        if(signedPts != ADM_NO_PTS && signedDts>signedPts)
		{
			// not sure it is correct. We may want to do it the other way around, i.e. bumping pts
			ADM_warning("Compensating for rounding error with PTS=%" PRId64"ms DTS=%" PRId64"ms \n",signedPts,signedDts);
                        signedPts=signedDts;
		}
	}
    }else
    {
// It means that the incoming image is earlier than the expected time.
// we add a bit of timeIncrement to compensate for rounding
        if(_nextFrameDts!=ADM_NO_PTS)
        {
            if(_nextFrameDts>(signedDts+vid->timeIncrementInUs/10))
            {
                ADM_error("Frame %" PRIu32" DTS is going back in time :");
                ADM_error("\t expected:%s ", ADM_us2plain(_nextFrameDts));
                ADM_error("\t got:%s \n", ADM_us2plain(signedDts));
                goto againGet;
            }
        }
        _nextFrameDts=signedDts;
    }
    // Increase for next one
    if(ADM_NO_PTS!=_nextFrameDts)
        _nextFrameDts+=vid->timeIncrementInUs;
    // Check the DTS is not too late compared to next seg beginning...
    if(_currentSegment+1<_segments.getNbSegments() && img->demuxerDts!=ADM_NO_PTS)
    {
        _SEGMENT *nextSeg=_segments.getSegment(_currentSegment+1);
        int64_t nextDts=nextSeg->_startTimeUs+nextSeg->_refStartDts;
        if(nextDts<nextSeg->_refStartTimeUs)
        {
            ADM_warning("%" PRIu32" next DTS is negative %" PRIu64" %" PRIu64" ms\n",fn,nextDts,nextSeg->_refStartTimeUs);
        }else       
        {
            nextDts-=nextSeg->_refStartTimeUs;
            if(signedDts>=nextDts)
            {
                ADM_warning("%" PRIu32" have to switch segment, DTS limit reached %" PRIu64" %" PRIu64"\n",fn,img->demuxerDts/1000,nextDts/1000);
                goto nextSeg;
            }
        }


    }
// **
   // ADM_info("Frame after RECAL: Flags :%X, DTS:%"PRId64" PTS=%"PRId64" tail=%"PRId64"\n",img->flags,img->demuxerDts/1000,img->demuxerPts/1000,tail);
    {
    int64_t finalDts=signedDts+(int64_t)videoDelay;
    if(finalDts<0) 
    {
        ADM_warning("Final DTS <0, dropping it \n");
        finalDts=0;
        goto againGet;
    }
    img->demuxerDts=finalDts;
    }
    img->demuxerPts=signedPts+(int64_t)videoDelay;
    aprintf("Final Pts=%s ",ADM_us2plain(img->demuxerPts));
    aprintf("Final Dts=%s ",ADM_us2plain(img->demuxerDts));    
    aprintf("Delay =%s ",ADM_us2plain(videoDelay));    
    return true;

nextSeg:
    if(false==switchToNextSegment(true))
    {
        ADM_warning("Cannot update to new segment\n");
        return false;
    }
    // Mark it as drop b frames...
    _SEGMENT *thisseg=_segments.getSegment(_currentSegment);
    thisseg->_dropBframes=ADM_DROP_MAYBE_AFER_SWITCH;
    ADM_info("Retrying for next segment\n");
    return getCompressedPicture(videoDelay,img);
   
}

/**
        \fn     getDirectImage
        \brief  For DEBUG DO NOT USE!

*/
bool        ADM_Composer::getDirectImageForDebug(uint32_t frameNum,ADMCompressedImage *img)
{
  
    _SEGMENT *seg=_segments.getSegment(0);
    ADM_assert(seg);
    _VIDEOS *vid=_segments.getRefVideo(seg->_reference);
    ADM_assert(vid);
    vidHeader *demuxer=vid->_aviheader;
    ADM_assert(demuxer);

    // Get next pic?
    if(false==demuxer->getFrame (frameNum,img))
    {
        ADM_info("Failed to get next frame for ref %" PRIu32"\n",seg->_reference);
        return false;
    }
   return true;
}
