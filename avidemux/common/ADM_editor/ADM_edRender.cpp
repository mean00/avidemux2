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
#include "ADM_default.h"
#include "ADM_editor/ADM_edit.hxx"

#if defined(ADM_DEBUG) && 0
#define aprintf printf
#else
#define aprintf(...) {}// printf
#endif

#include "ADM_pp.h"
/**
    \fn recalibrate
    \brief Convert time given in time from absolute ref video to linear time
*/
static void recalibrate(uint64_t *time,_SEGMENT *seg)
{
int64_t t=(int64_t)*time;
        if(*time==ADM_NO_PTS) return;

        t-=seg->_refStartTimeUs;
        if(t<0)
        {
            ADM_warning("Segment time is negative!\n");
            t=0;
        }
        t+=seg->_startTimeUs;
        *time=(uint64_t )t;
}
/**
    \fn updateImageTiming
*/
bool updateImageTiming(_SEGMENT *seg,ADMImage *image)
{
    recalibrate(&(image->Pts),seg);
//    recalibrate(&(image->Dts),seg);
    return true;
}
/**
    \fn GoToIntraTime
    \brief Go to an intra at time time (exact)
    \return true on success, false on error
*/
bool        ADM_Composer::goToIntraTimeVideo(uint64_t time)
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
    if(false==switchToSegment(s))
    {
        ADM_warning("Cannot go to segment %"LU"\n",s);
        return false;
    }
    if(false== DecodePictureUpToIntra(seg->_reference,frame))
    {
        return false;
    }
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
    uint64_t to=segTime+s->_refStartTimeUs;
    if(false==seektoTime(s->_reference,to))
    {
            ADM_warning("Cannot seek to beginning of segment %"LU" at  %"LLU" ms\n",s,to/1000);
            return false;
    }
    _currentSegment=seg;
    return true;

}
/**
    \fn NextPicture
    \brief decode & returns the next picture
*/
bool        ADM_Composer::nextPicture(ADMImage *image)
{
uint64_t pts;
uint64_t tail;

    // Decode image...
    _SEGMENT *seg=_segments.getSegment(_currentSegment);
    if(false== nextPictureInternal(seg->_reference,image))
    {
        goto np_nextSeg;
    }
        // Refresh in case we switched....
        seg=_segments.getSegment(_currentSegment);
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
        

        return true;

// Try to get an image for the following segment....
np_nextSeg:
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
            return true;
        } else  
        {
                ADM_warning("Cannot get next picture. Last segment\n");
                return false;
        }
        return false;
}
/**
    \fn samePicture
    \brief returns the last already decoded picture
*/
bool        ADM_Composer::samePicture(ADMImage *image)
{
      _SEGMENT *seg=_segments.getSegment(_currentSegment);
        if(false== samePictureInternal(seg->_reference,image))
        {
            ADM_warning("SamePicture failed\n");
            return false;
        }
         updateImageTiming(seg,image);
        return true;
}

/**
        \fn getCompressedPicture
        \brief bypass decoder and directly get the source image

*/
bool        ADM_Composer::getCompressedPicture(ADMCompressedImage *img)
{
    uint64_t tail;
    //
    _SEGMENT *seg=_segments.getSegment(_currentSegment);
    ADM_assert(seg);
    _VIDEOS *vid=_segments.getRefVideo(seg->_reference);
    ADM_assert(vid);
    vidHeader *demuxer=vid->_aviheader;
    ADM_assert(demuxer);

    // Get next pic?
    if(false==demuxer->getFrame (seg->_curFrame,img))
    {
        ADM_info("Failed to get next frame for ref %"LU"\n",seg->_reference);
        goto nextSeg;
    }
    seg->_curFrame++;
    // Need to switch seg ?
    tail=seg->_refStartTimeUs+seg->_durationUs;
    if(img->demuxerDts!= ADM_NO_PTS && img->demuxerDts>tail) goto nextSeg;
    if(img->demuxerPts!= ADM_NO_PTS && img->demuxerPts>tail) goto nextSeg;
    {
    // Recalibrate PTS & DTS...
    int64_t offset=seg->_refStartTimeUs;
    offset=seg->_startTimeUs;
    recalibrate(&(img->demuxerPts),seg);
    recalibrate(&(img->demuxerDts),seg);
    }
    return true;

nextSeg:
    if(false==switchToNextSegment())
    {
        ADM_warning("Cannot update to new segment\n");
        return false;
    }
    ADM_info("Retrying for next segment\n");
    return getCompressedPicture(img);
   
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
                                vid->color=new COL_Generic2YV12(src->_width,src->_height,src->_colorspace);
                        }
                        // Since it is not YV12 it MUST be a ref
                        ADM_assert(src->_isRef);
                        vid->color->transform(src->_planes,src->_planeStride,dst->data);
                        return 1;
                }
                // nothing to do
                if(_pp.swapuv)
                      dst->duplicateSwapUV(src);
                else
                        dst->duplicate(src);
                return 1;
}
/**
    \fn setPostProc
*/
uint8_t ADM_Composer::setPostProc( uint32_t type, uint32_t strength, uint32_t swapuv)
{
	if(!_segments.getNbRefVideos()) return 0;
	_pp.postProcType=type;
	_pp.postProcStrength=strength;
        _pp.swapuv=swapuv;
	updatePostProc(&_pp); // DeletePostproc/ini missing ?
	return 1;
}
/**
    \fn getPostProc
*/

uint8_t ADM_Composer::getPostProc( uint32_t *type, uint32_t *strength, uint32_t *swapuv)
{
	if(!_segments.getNbRefVideos()) return 0;
	*type=_pp.postProcType;
	*strength=_pp.postProcStrength;
	*swapuv=_pp.swapuv;
	return 1;
}
/**
    \fn switchToNextSegment
    \brief Switch to the next segment

*/
bool        ADM_Composer::switchToNextSegment(void)
{
    if(_currentSegment==_segments.getNbSegments()-1)
    {
        ADM_warning("This is the last segment (%"LU")\n",_currentSegment);
        return false;
    }
    if(true==switchToSegment(_currentSegment+1)) return true;
    return false;
}
/**
    \fn switchToSegment
    \brief Switch to the segment given as argument

*/
bool        ADM_Composer::switchToSegment(uint32_t s)
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
      uint32_t flags;
      uint64_t pts,dts;
        demuxer->getFlags(0,&flags);
        demuxer->getPtsDts(0,&pts,&dts);
    if(!seg->_refStartTimeUs)
    {
        if(pts!=ADM_NO_PTS && pts)
        {
                ADM_info("This segment does not start at 0,...\n");
                from=pts;
        }
    }
    if(false==seektoTime(seg->_reference,from))
    {
            ADM_warning("Cannot seek to beginning of segment %"LU" at  %"LLU" ms\n",s,from/1000);
            return false;
    }
    _currentSegment=s;
    ADM_info("Switched ok to segment %"LU"\n",s);
    return true;
}
/**
    \fn rewind
    \brief
*/
bool ADM_Composer::rewind(void)
{
        ADM_info("Rewinding\n");
        return DecodePictureUpToIntra(0,0);

}
//EOF
