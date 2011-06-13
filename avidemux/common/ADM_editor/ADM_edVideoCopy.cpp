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
#include "ADM_editor/ADM_edit.hxx"
#include "ADM_vidMisc.h"
#if defined(ADM_DEBUG) && 0
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
    uint8_t buffer[1920*1080*3];
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
            fail=true;
            break;
        }
        if(img.flags & AVI_KEY_FRAME)
        {
            ADM_info("Segment %d starts on a keyframe\n",i);
        }else   
        {
            ADM_warning("Segment %d does not start on a keyframe (%s)\n",i,ADM_us2plain(img.demuxerPts));
            fail=true;
             break;
        }
    }   

    return fail;
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
        ADM_info("Failed to get next frame for ref %"LU"\n",seg->_reference);
        goto nextSeg;
    }

    vid->lastSentFrame++;
    //
    if(img->flags & AVI_B_FRAME)
    {
        if(seg->_dropBframes==2) 
        {
            ADM_warning("%"LU" Dropping bframes\n",fn);
            goto againGet;
        }
    }else
    { // not a bframe
        switch(seg->_dropBframes)
        {
            case 2: seg->_dropBframes=0;break;
            case 1: seg->_dropBframes=2;break;
            default: break;
        }
    }
    // after a segment switch, we may have some frames from "the past"
    // if the cut point is not a keyframe, drop them
#if 1
    if(_currentSegment && img->demuxerDts!=ADM_NO_PTS)
    {
        if(img->demuxerDts<seg->_refStartDts)
        {
            ADM_info("Frame %d is in the past for this segment (%s)",vid->lastSentFrame,ADM_us2plain(img->demuxerPts));
            ADM_info("vs %s\n",ADM_us2plain(seg->_refStartDts));
            goto againGet;
        }
    }
#endif
    // Need to switch seg ?
    tail=seg->_refStartTimeUs+seg->_durationUs;
    // Guess DTS
    //
   // ADM_info("Frame : Flags :%X, DTS:%"LLD" PTS=%"LLD" tail=%"LLD"\n",img->flags,img->demuxerDts/1000,img->demuxerPts/1000,tail);
    if(img->demuxerDts!= ADM_NO_PTS && img->demuxerDts>=tail) goto nextSeg;
    if(img->demuxerPts!= ADM_NO_PTS && img->demuxerPts>=tail) goto nextSeg;
    
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
    
    // From here we are in linear time...
    if(img->demuxerDts==ADM_NO_PTS)
    {
        img->demuxerDts=_nextFrameDts;
        signedDts=_nextFrameDts;
    }else
    {
// It means that the incoming image is earlier than the expected time.
// we add a bit of timeIncrement to compensate for rounding
        if(_nextFrameDts!=ADM_NO_PTS)
        {
            if(_nextFrameDts>signedDts+vid->timeIncrementInUs/10)
            {
                ADM_error("Frame %"LU" DTS is going back in time : expected : %"LLD" ms got : %"LLD" ms\n",
                                                fn,_nextFrameDts/1000,signedDts/1000);
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
            ADM_warning("%"LU" next DTS is negative %"LLU" %"LLU" ms\n",fn,nextDts,nextSeg->_refStartTimeUs);
        }else       
        {
            nextDts-=nextSeg->_refStartTimeUs;
            if(signedDts>=nextDts)
            {
                ADM_warning("%"LU" have to switch segment, DTS limit reached %"LLU" %"LLU"\n",fn,img->demuxerDts/1000,nextDts/1000);
                goto nextSeg;
            }
        }


    }
   // ADM_info("Frame after RECAL: Flags :%X, DTS:%"LLD" PTS=%"LLD" tail=%"LLD"\n",img->flags,img->demuxerDts/1000,img->demuxerPts/1000,tail);
    img->demuxerDts=signedDts+videoDelay;
    img->demuxerPts=signedPts+videoDelay;
    return true;

nextSeg:
    if(false==switchToNextSegment(true))
    {
        ADM_warning("Cannot update to new segment\n");
        return false;
    }
    // Mark it as drop b frames...
    _SEGMENT *thisseg=_segments.getSegment(_currentSegment);
    thisseg->_dropBframes=1;
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
        ADM_info("Failed to get next frame for ref %"LU"\n",seg->_reference);
        return false;
    }
   return true;
}
