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
#include <math.h>
#include "ADM_default.h"
#include "ADM_editor/ADM_edit.hxx"

#if defined(ADM_DEBUG) && 0
#define aprintf printf
#else
#define aprintf(...) {}// printf
#endif

/**
        \fn getCompressedPicture
        \brief bypass decoder and directly get the source image

    The dropBframe is as follow :
            0 : Dont drop b frame
            1 : Follow the next bframes
            2 : Drop


*/
bool        ADM_Composer::getCompressedPicture(ADMCompressedImage *img)
{
    uint64_t tail;
    //
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
    // Need to switch seg ?
    tail=seg->_refStartTimeUs+seg->_durationUs;
    // Guess DTS

   // ADM_info("Frame : Flags :%X, DTS:%"LLD" PTS=%"LLD" tail=%"LLD"\n",img->flags,img->demuxerDts/1000,img->demuxerPts/1000,tail);
    if(img->demuxerDts!= ADM_NO_PTS && img->demuxerDts>=tail) goto nextSeg;
    if(img->demuxerPts!= ADM_NO_PTS && img->demuxerPts>=tail) goto nextSeg;
    {
        // Recalibrate PTS & DTS...
        recalibrate(&(img->demuxerPts),seg);
        recalibrate(&(img->demuxerDts),seg);
    }
    // From here we are in linear time...
    if(img->demuxerDts==ADM_NO_PTS)
    {
        img->demuxerDts=_nextFrameDts;
    }else
    {
// It means that the incoming image is earlier than the expected time.
// we add a bit of timeIncrement to compensate for rounding
        if(_nextFrameDts>img->demuxerDts+vid->timeIncrementInUs/10)
        {
         ADM_error("Frame %"LU" DTS is going back in time : expected : %"LLU" ms got : %"LLU" ms\n",fn,_nextFrameDts/1000,img->demuxerDts/1000);
        }
        _nextFrameDts=img->demuxerDts;
    }
    // Increase for next one
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
            if(img->demuxerDts>=nextDts)
            {
                ADM_warning("%"LU" have to switch segment, DTS limit reached %"LLU" %"LLU"\n",fn,img->demuxerDts/1000,nextDts/1000);
                goto nextSeg;
            }
        }


    }
   // ADM_info("Frame after RECAL: Flags :%X, DTS:%"LLD" PTS=%"LLD" tail=%"LLD"\n",img->flags,img->demuxerDts/1000,img->demuxerPts/1000,tail);
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
    return getCompressedPicture(img);
   
}
