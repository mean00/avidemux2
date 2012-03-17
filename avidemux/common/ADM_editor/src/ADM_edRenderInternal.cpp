/***************************************************************************
    \file  ADM_edRenderInternal.cpp  
    \brief handle decoding from ONE source, ignoring the segments
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

#if defined(ADM_DEBUG) && 0
#define aprintf printf
#else
#define aprintf(...) {}// printf
#endif

#include "ADM_pp.h"
// FIXME BADLY !!!
// This should be in a context somewhere
static uint8_t compBuffer[MAXIMUM_SIZE * MAXIMUM_SIZE * 3];

/**
    \fn seektoFrame
    \brief Seek to frame with timestamp given as arg

*/
bool ADM_Composer::seektoTime(uint32_t ref,uint64_t timeToSeek,bool dontdecode)
{
   _VIDEOS *vid=_segments.getRefVideo(ref);
    vidHeader *demuxer=vid->_aviheader;
	EditorCache   *cache =vid->_videoCache;
	ADM_assert(cache);
    bool found=false;
   // Search the previous keyframe for segment....
    uint64_t seekTime;
    if(_segments.isKeyFrameByTime(ref,timeToSeek))
    {
        seekTime=timeToSeek;
        ADM_info("First frame of the new segment is a keyframe at %"LU"ms\n",seekTime/1000);
        found=true;
    }else   
    {
        if(false==searchPreviousKeyFrameInRef(ref,timeToSeek,&seekTime))
        {
            ADM_warning("Cannot identify the keyframe before %"LLU" ms\n",seekTime/1000);
            return false;
        }
    }
    uint32_t frame=_segments.intraTimeToFrame(ref,seekTime);
    if(dontdecode==true)
    {
        vid->lastSentFrame=frame;
        ADM_info("Seek to time without decoding ok\n");
        return true;
    }
    // ok now seek...
    
    if(false==DecodePictureUpToIntra(ref,frame))
    {
        ADM_warning("Cannot decode up to intra %"LLU" at %"LLU" ms\n",frame,seekTime/1000);
        return false;
    }
    if(found==true) return true;
    // Now forward...
    while(nextPictureInternal(ref,NULL)==true)
    {
        uint64_t pts=vid->lastDecodedPts;
        if(pts==ADM_NO_PTS)     
        {
            ADM_warning("No PTS out of decoder\n");
            continue;
        }
        vid->lastReadPts=pts;
        if(pts==timeToSeek)
        {
            ADM_info("Image found, pts=%"LLU" ms\n",pts/1000);
            return true;
        }
        if(pts>timeToSeek)
        {
            ADM_info("Image not found,searching %"LLU" ms, got  pts=%"LLU" ms\n",timeToSeek/1000,pts/1000);
            return false;
        }
    }
    ADM_warning("seekToFrame failed for frame at PTS= %"LLU" ms, next image failed\n",timeToSeek/1000);
    return false;
}
/**
    \fn samePictureInternal
    \brief returns the last already decoded picture
    @param out : Where to put the decoded image to
    @param ref : Video we are dealing with
    @return true on success, false on failure

*/
bool ADM_Composer::samePictureInternal(uint32_t ref,ADMImage *out)
{
    _VIDEOS *vid=_segments.getRefVideo(ref);
    vidHeader *demuxer=vid->_aviheader;
	EditorCache   *cache =vid->_videoCache;
	ADM_assert(cache);

  ADMImage *in=cache->getByPts(vid->lastDecodedPts);
  if(!in)
  {
    printf("[ADM_Composer::getSamePicture] Failed, while looking for Pts=%"LLU" ms\n",vid->lastDecodedPts);
    cache->dump();
    return false;
  }
  out->duplicate(in);
  return true;
}

/**
    \fn nextPictureInternal
    \brief returns the next picture
    @param out : Where to put the decoded image to
    @param ref : Video we are dealing with
    @return true on success, false on failure

*/
bool ADM_Composer::nextPictureInternal(uint32_t ref,ADMImage *out)
{
  ADMImage	*result;
  
  _VIDEOS *vid=_segments.getRefVideo(ref);
  EditorCache   *cache=vid->_videoCache;
  ADM_assert(vid);

   uint32_t loop=20; // Try 20 frames ahead

	// Try decoding loop rames ahead, if not we can consider it fails
    while(loop--)
    {
        // first decode a picture, cannot hurt...
        if(DecodeNextPicture(ref)==false)
        {
            ADM_warning("Next picture failed\n");
            continue;
        }
        // Search the lowest PTS above our current PTS...
        ADMImage *img=cache->getAfter(vid->lastReadPts);
        if(img)
        {
            // Duplicate
            if(out)
            {
                aprintf("[getNextPicture] Looking for after> %"LLU", got %"LLU" delta=%"LD" ms\n",vid->lastReadPts,img->Pts,(img->Pts-vid->lastReadPts)/1000);
                out->duplicate(img);
                vid->lastReadPts=img->Pts;
                currentFrame++;
            }
            return true;
        }else   
        {
            aprintf("[getNextPic] Loop:%d, looking for pts> :%"LLU" ms %"LLU" us\n",loop,vid->lastReadPts/1000,vid->lastReadPts);
#ifdef VERBOSE
            cache->dump();
#endif

        }
    }
    ADM_warning("nextPictureInternal Failed\n");
    ADM_warning("while looking for %"LLU" us, %"LLU" ms\n",vid->lastReadPts,vid->lastReadPts/1000);
    cache->dump();
    return false;
}

/**
    \fn DecodeNextPicture
    \brief Decode the next picture

    @param ref  , video we are dealing with
    returns true on success
            fail on error

*/
bool ADM_Composer::DecodeNextPicture(uint32_t ref)
{
uint8_t ret = 0;
  EditorCache   *cache;
  ADMImage	*result;
  uint32_t  flags;
  ADMCompressedImage img;
   _VIDEOS *vid=_segments.getRefVideo(ref);
    vidHeader *demuxer=vid->_aviheader;
	cache=vid->_videoCache;
    // PlaceHolder...
    img.data=compBuffer;
    img.cleanup(vid->lastSentFrame+1);

	ADM_assert(cache);
    vid->lastSentFrame++;

    uint32_t frame=vid->lastSentFrame;
    aprintf("[EditorRender] DecodeNext %u ref:%u\n",frame,ref);
    // Fetch frame
     aprintf("[Editor] Decoding frame %u\n",frame);
     if (!demuxer->getFrame (frame,&img))
     {
            ADM_warning("getFrame failed for frame %"LU"\n",vid->lastSentFrame);
            return false;
     }

     // Now uncompress it...
     result=cache->getFreeImage();
     if(!result)
     {
            ADM_warning(" Cache full for frame %"LU"\n",vid->lastSentFrame);
            return false;
      }
        aprintf("Demuxed frame %"LU" with pts=%"LLD" us, %"LLD" ms\n",
            frame,
            img.demuxerPts,
            img.demuxerPts/1000);
    
      if(!decompressImage(result,&img,ref))
      {
         ADM_info("Decoding error for frame %"LU", not necessarily a problem\n",vid->lastSentFrame);
         cache->invalidate(result);
         return true; // Not an error in itself
      }
     
     uint64_t pts=result->Pts;
     uint64_t old=vid->lastDecodedPts;
        if(pts==ADM_COMPRESSED_NO_PTS) // No PTS available ?
        {
                aprintf("[Editor] No PTS, guessing value\n");
                vid->lastDecodedPts+=vid->timeIncrementInUs;
                result->Pts=vid->lastDecodedPts;
        }else
           {
                aprintf("[Editor] got PTS\n");
                vid->lastDecodedPts=pts;
            }
    aprintf(">>Decoded frame %"LU" with pts=%"LLD" us, %"LLD" ms, ptsdelta=%"LLD" ms \n",
        frame,
        vid->lastDecodedPts,
        vid->lastDecodedPts/1000,
        (vid->lastDecodedPts-old)/1000);

    if(old>vid->lastDecodedPts) 
    {
        ADM_warning(">>>>> PTS going backward by %"LLD" ms\n",(old-vid->lastDecodedPts)/1000);
        ADM_warning("Dropping frame!\n");
        cache->invalidate(result);
        return false;
    }else
    {
        cache->validate(result);
    }
    return true;
}


/**
    \fn decompressImage
    \brief push an image inside decoder and pop one. Warning the popped one might be an older image due to decoder lag.
            Also do postprocessing and color conversion
*/
bool ADM_Composer::decompressImage(ADMImage *out,ADMCompressedImage *in,uint32_t ref)
{
 ADMImage *tmpImage=NULL;
 _VIDEOS  *v=_segments.getRefVideo(ref);
 uint32_t refOnly=v->decoder->dontcopy(); // can we skip one memcpy ?
// This is only an empty Shell
    if(refOnly)
    {
                uint32_t w,h;
                if(_scratch) // Can we reuse the old scratch memory ?
                {
                    _scratch->getWidthHeight(&w,&h);
                    if(w!=_imageBuffer->_width || _imageBuffer->_height!=h)
                    {
                        delete _scratch;
                        _scratch=NULL;
                    }
                }
                if(!_scratch)
                {
                  _scratch=new ADMImageRef(_imageBuffer->_width,_imageBuffer->_height);
                }
                tmpImage=_scratch;
        }
        else
        {
                tmpImage=_imageBuffer;
       }
    //
    tmpImage->_colorspace=ADM_COLOR_YV12;
	// Decode it
        if (!v->decoder->uncompress (in, tmpImage))
	    {
            printf("[decompressImage] uncompress failed\n");
            return false;
        }

        //
        if(tmpImage->_noPicture && refOnly)
        {
            printf("[decompressImage] NoPicture\n");
            // Fill in with black
            return true;
        }
        aprintf("[::Decompress] in:%"LU" out:%"LU" flags:%x\n",in->demuxerPts,out->Pts,out->flags);
	// If not quant and it is already YV12, we can stop here
    // Also, if the image is decoded through hw, dont do post proc
	if(tmpImage->refType!=ADM_HW_NONE || 
                    (!tmpImage->quant || !tmpImage->_qStride) && tmpImage->_colorspace==ADM_COLOR_YV12)
	{
		out->_Qp=2;
		out->duplicate(tmpImage);
		aprintf("[decompressImage] : No quant avail\n");
		return true;
	}
	// We got everything, let's go
	// 1 compute average quant
	int qz;
	uint32_t sumit=0;
    // Dupe infos
    out->copyInfo(tmpImage);


    // Do postprocessing if any
	for(uint32_t z=0;z<tmpImage->_qSize;z++)
	{
            qz=(int)tmpImage->quant[z];
			sumit+=qz;
	}
	sumit+=(tmpImage->_qSize-1);
	float sum=(float)sumit;
	sum/=tmpImage->_qSize;
	if(sum>31) sum=31;
	if(sum<1) sum=1;

    // update average Q
	tmpImage->_Qp=out->_Qp=(uint32_t)floor(sum);

	// Pp deactivated ?
	if(!_pp->postProcType || !_pp->postProcStrength || tmpImage->_colorspace!=ADM_COLOR_YV12)
    {
        dupe(tmpImage,out,v);
        aprintf("EdCache: Postproc disabled\n");
		return 1;
	}
    /* Do it!*/
    _pp->process(tmpImage,out);
    return true;
}

/**
    \fn DecodePictureUpToIntra
    \brief Decode pictures from frameno, which must be an intra and on
            until the decoded frameno is popped by the decoder

    @param frame, framenumber relative to video ref (i.e. from its beginning)
    @param ref  , video we are dealing with
    returns true on success
            fail on error

*/
bool ADM_Composer::DecodePictureUpToIntra(uint32_t ref,uint32_t frame)
{
  uint8_t ret = 0;
  EditorCache   *cache;
  ADMImage	*result;
  uint32_t  flags;
  ADMCompressedImage img;

    // PlaceHolder...
    img.data=compBuffer;
    img.cleanup(frame);

    ADM_info(" DecodeUpToInta %u ref:%u\n",frame,ref);
	_VIDEOS *vid=_segments.getRefVideo(ref);
    vidHeader *demuxer=vid->_aviheader;
	cache=vid->_videoCache;
	ADM_assert(cache);
    // Make sure frame is an intra
    demuxer->getFlags(frame,&flags);
    ADM_assert(flags&AVI_KEY_FRAME);

    bool found=false;
    vid->lastSentFrame=frame;
    uint32_t nbFrames=vid->_nb_video_frames;
    aprintf("[EditorRender] DecodeUpToIntra flushing cache & codec\n");
    cache->flush();
    vid->decoder->flush();
    // The PTS associated with our frame is the one we are looking for
    uint64_t wantedPts=demuxer->estimatePts(frame);
    uint32_t tries=15+7; // Max Ref frames for H264 + MaxRecovery , let's say 7 is ok for recovery
    bool syncFound=false;
    while(found==false && tries--)
    {
        // Last frame ? if so repeat
        if(vid->lastSentFrame>=nbFrames-1) vid->lastSentFrame=nbFrames-1;
        // Fetch frame
         aprintf("[Editor] Decoding  frame %u\n",vid->lastSentFrame);
         
         if (!demuxer->getFrame (vid->lastSentFrame,&img))
         {
                ADM_warning(" getFrame failed for frame %"LU"\n",vid->lastSentFrame);
                //cache->flush();
                return false;
         }
         // Now uncompress it...
         result=cache->getFreeImage();
         if(frame==0) // out first frame, make sure it starts black to avoid the all green effect
         {
            result->blacken();
         }
         if(!result)
         {
                ADM_warning(" Cache full for frame %"LU"\n",vid->lastSentFrame);
                return false;
          }
           aprintf("[Decoder] Demuxer Frame %"LU" pts=%"LLU" ms, %"LLU" us\n",vid->lastSentFrame,img.demuxerPts/1000,
                                                                    img.demuxerPts);
          if(!decompressImage(result,&img,ref))
          {
             ADM_info(" decode error for frame %"LU", not necessarily a problem\n",vid->lastSentFrame);
             //cache->dump();
             cache->invalidate(result);
             //cache->dump();
             vid->lastSentFrame++;
             continue;
          }else
            {
                
                uint64_t pts=result->Pts;
                aprintf("[Decoder] Decoder Frame %"LU" pts=%"LLU" ms, %"LLU" us\n",vid->lastSentFrame,
                                                        result->Pts/1000,result->Pts);
                if(pts==ADM_COMPRESSED_NO_PTS) // No PTS available ?
                {
                   
                    if(false==syncFound)
                    {
                        aprintf("[DecodePictureUpToIntra] No time stamp yet, dropping picture\n");
                        cache->invalidate(result);
                    }else
                    {
                        // increment it using average fps
                        vid->lastDecodedPts+=vid->timeIncrementInUs;
                        result->Pts=vid->lastDecodedPts;
                    }
                }else
                {
                    if(false==syncFound)
                    {
                        aprintf("[DecodePictureUpToIntra] Sync found\n");
                        syncFound=true;
                    }
                    vid->lastDecodedPts=pts;
                }
                cache->validate(result);
            }

            // Found our image ?
          if(result->Pts==wantedPts)
                found=true;
          else
                vid->lastSentFrame++;
    }
    if(found==false)
    {
        ADM_warning(" Could not find decoded frame, wanted PTS :%"LU" PTS=%"LLU" ms, %"LLU" us\n",frame,wantedPts/1000,wantedPts);
        cache->dump();
        return false;
    }
    vid->lastReadPts=wantedPts;
    currentFrame=frame;
    return true;
}
//EOF
