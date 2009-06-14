/***************************************************************************
                          ADM_edRender.cpp  -  description
                             -------------------
     This file renders the compressed video depending on the availabilty of
     		CODECs.      It also deals with key frame and stuff


    begin                : Fri Apr 12 2002
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
#include <math.h>
#include "ADM_default.h"
#include "ADM_editor/ADM_edit.hxx"

#if defined(ADM_DEBUG) && 0
#define aprintf printf
#else
#define aprintf(...) {}// printf
#endif

#include "ADM_pp.h"

// FIXME BADLY !!!
static uint8_t compBuffer[MAXIMUM_SIZE * MAXIMUM_SIZE * 3];
/**
        \fn GoToIntra
        \brief Go to frame which must be an intra
        @param frame is framenumber as seen by user
        \return true on success, false on error
*/
bool        ADM_Composer::GoToIntra(uint32_t frame)
{
uint32_t relframe, seg, flags, len;

  // convert frame to block, relative frame
  if (!convFrame2Seg (frame, &seg, &relframe))
    {
      printf ("[ADMComposer::goToIntra] Conversion failed for frame %"LU" !\n",frame);
      return false;
    }
    return DecodePictureUpToIntra(relframe,seg);
}
/**
    \fn GoToTime
    \brief Go to time time (just before)
    \return true on success, false on error
*/
bool        ADM_Composer::GoToTime(uint64_t time)
{
    // 1st go to the previous intra...
      uint32_t frame=searchFrameBefore(time+1);
      while(frame)
      {
            uint32_t flags;
                getFlags(frame,&flags);
                if(flags & AVI_KEY_FRAME) break;
                frame--;
      }
        // Now go forward if needed
#warning todo fixme!
    printf("[Composer:GoToTime] Going to frame %"LU"\n",frame);
    return GoToIntra(frame);
}
/**
    \fn NextPicture
    \brief decode & returns the next picture
*/
bool        ADM_Composer::NextPicture(ADMImage *image)
{
    return getNextPicture(image,0);

}
/**
    \fn samePicture
    \brief returns the last already decoded picture
*/
bool        ADM_Composer::samePicture(ADMImage *image)
{
    return getSamePicture(image,0);

}
/**
        \fn getCompressedPicure
        \brief bypass decoder and directly get the source image

*/
bool        ADM_Composer::getCompressedPicure(uint32_t framenum,ADMCompressedImage *img)
{
uint8_t ref = 0;

    _VIDEOS *vid=&_videos[ref];
    vidHeader *demuxer=vid->_aviheader;

    img->cleanup(framenum);

    if (!demuxer->getFrame (framenum,img)) return false;
    return true;
}
//***************************** Internal API**************************
/**
    \fn DecodePictureUpToIntra
    \brief Decode pictures from frameno, which must be an intra and on
            until the decoded frameno is popped by the decoder

    @param frame, framenumber relative to video ref (i.e. from its beginning)
    @param ref  , video we are dealing with
    returns true on success
            fail on error

*/
bool ADM_Composer::DecodePictureUpToIntra(uint32_t frame,uint32_t ref)
{
  uint8_t ret = 0;
  EditorCache   *cache;
  ADMImage	*result;
  uint32_t  flags;
  ADMCompressedImage img;

    // PlaceHolder...
    img.data=compBuffer;
    img.cleanup(frame);

    printf("[EditorRender] DecodeUpToInta %u ref:%u\n",frame,ref);
	_VIDEOS *vid=&_videos[ref];
    vidHeader *demuxer=vid->_aviheader;
	cache=_videos[ref]._videoCache;
	ADM_assert(cache);
    // Make sure frame is an intra
    demuxer->getFlags(frame,&flags);
    ADM_assert(flags==AVI_KEY_FRAME);

    bool found=false;
    vid->lastSentFrame=frame;
    uint32_t nbFrames=vid->_nb_video_frames;
    aprintf("[EditorRender] DecodeUpToIntra flushing cache & codec\n");
    cache->flush();
    _videos[ref].decoder->flush();
    // The PTS associated with our frame is the one we are looking for
    uint64_t wantedPts=vid->_aviheader->estimatePts(frame);
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
                printf("[DecodePictureUpToIntra] getFrame failed for frame %"LU"\n",vid->lastSentFrame);
                //cache->flush();
                return false;
         }
         // Now uncompress it...
         result=cache->getFreeImage();
         if(!result)
         {
                printf("[DecodePictureUpToIntra] Cache full for frame %"LU"\n",vid->lastSentFrame);
                return false;
          }
           aprintf("[Decoder] Demuxer Frame %"LU" pts=%"LLU" ms, %"LLU" us\n",vid->lastSentFrame,img.demuxerPts/1000,
                                                                    img.demuxerPts);
          if(!decompressImage(result,&img,ref))
          {
             printf("[DecodePictureUpToIntra] decode error for frame %"LU"\n",vid->lastSentFrame);
             //cache->dump();
             cache->invalidate(result);
             //cache->dump();
             vid->lastSentFrame++;
             continue;
          }else
            {
                cache->updateFrameNum(result,vid->lastSentFrame);
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
            }

            // Found our image ?
          if(result->Pts==wantedPts)
                found=true;
          else
                vid->lastSentFrame++;
    }
    if(found==false)
    {
        printf("[GoToIntra] Could not find decoded frame, wanted PTS :%"LU" PTS=%"LLU" ms, %"LLU" us\n",frame,wantedPts/1000,wantedPts);
        cache->dump();
        return false;
    }
    vid->lastReadPts=wantedPts;
    currentFrame=frame;
    return true;
}
/**
    \fn getSamePicture
    \brief returns the last already decoded picture
    @param out : Where to put the decoded image to
    @param ref : Video we are dealing with
    @return true on success, false on failure

*/
bool ADM_Composer::getSamePicture(ADMImage *out,uint32_t ref)
{
    _VIDEOS *vid=&_videos[ref];
    vidHeader *demuxer=vid->_aviheader;
	EditorCache   *cache =_videos[ref]._videoCache;
	ADM_assert(cache);

  ADMImage *in=cache->getByPts(vid->lastDecodedPts);
  if(!in)
  {
    printf("[ADM_Composer::getSamePicture] Failed\n");
    return false;
  }
  out->duplicate(in);
  return true;
}

/**
    \fn getNextPicture
    \brief returns the next picture
    @param out : Where to put the decoded image to
    @param ref : Video we are dealing with
    @return true on success, false on failure

*/
bool ADM_Composer::getNextPicture(ADMImage *out,uint32_t ref)
{
  EditorCache   *cache=_videos[ref]._videoCache;
  ADMImage	*result;
  _VIDEOS *vid=&_videos[ref];

   uint32_t loop=20; // Try 20 frames ahead

	// Try decoding loopŔ rames ahead, if not we can consider it fails
    while(loop--)
    {
        // first decode a picture, cannot hurt...
        if(DecodeNextPicture(ref)==false)
        {
            printf("[AdmComposer::getPicture] Next picture failed\n");
            continue;
        }
        // Search the lowest PTS above our current PTS...
        ADMImage *img=cache->findJustAfter(vid->lastReadPts);
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
    printf("[ADM_Composer::getPicture] Failed\n");
    printf("[ADM_composer] while looking for %"LLU" us, %"LLU" ms\n",vid->lastReadPts,vid->lastReadPts/1000);
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
   _VIDEOS *vid=&_videos[ref];
    vidHeader *demuxer=vid->_aviheader;
	cache=_videos[ref]._videoCache;
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
            printf("[DecodePictureUpToIntra] getFrame failed for frame %"LU"\n",vid->lastSentFrame);
            return false;
     }

     // Now uncompress it...
     result=cache->getFreeImage();
     if(!result)
     {
            printf("[DecodePictureUpToIntra] Cache full for frame %"LU"\n",vid->lastSentFrame);
            return false;
      }
        aprintf("Demuxed frame %"LU" with pts=%"LLD" us, %"LLD" ms\n",
            frame,
            img.demuxerPts,
            img.demuxerPts/1000);
    
      if(!decompressImage(result,&img,ref))
      {
         printf("[DecodePictureUpToIntra] decode error for frame %"LU"\n",vid->lastSentFrame);
         cache->invalidate(result);
         return true; // Not an error in itself
      }
     cache->updateFrameNum(result,vid->lastSentFrame);
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
    if(old>vid->lastDecodedPts) printf(">>>>> PTS going backward by %"LLD" ms\n",(old-vid->lastDecodedPts)/1000);
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
 uint32_t ww,hh,left,right;
 uint32_t refOnly=_videos[ref].decoder->dontcopy(); // can we skip one memcpy ?
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
                  _scratch=new ADMImage(_imageBuffer->_width,_imageBuffer->_height,1);
                }
                tmpImage=_scratch;
                ww=_imageBuffer->_width & 0xfffff0;
                left=_imageBuffer->_width & 0xf;

        }
        else
        {
                tmpImage=_imageBuffer;
                ww=_imageBuffer->_width;
                left=0;
       }
    //
    tmpImage->_colorspace=ADM_COLOR_YV12;
	// Decode it
        if (!_videos[ref].decoder->uncompress (in, tmpImage))
	    {
            printf("[decompressImage] uncompress failed\n");
            return false;
        }

        //
        if(tmpImage->_noPicture && refOnly)
        {
            printf("[decompressImage] NoPicture\n");
            return false;
        }
        aprintf("[::Decompress] in:%"LU" out:%"LU" flags:%x\n",in->demuxerPts,out->Pts,out->flags);
	// If not quant and it is already YV12, we can stop here
	if((!tmpImage->quant || !tmpImage->_qStride) && tmpImage->_colorspace==ADM_COLOR_YV12)
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
	if(!_pp.postProcType || !_pp.postProcStrength || tmpImage->_colorspace!=ADM_COLOR_YV12)
    {
        dupe(tmpImage,out,&(_videos[ref]));
        aprintf("EdCache: Postproc disabled\n");
		return 1;
	}

	int type;
	#warning FIXME should be FF_I_TYPE/B/P
	if(tmpImage->flags & AVI_KEY_FRAME) type=1;
		else if(tmpImage->flags & AVI_B_FRAME) type=3;
			else type=2;

        ADM_assert(tmpImage->_colorspace==ADM_COLOR_YV12);

	// we do postproc !
	// keep
	uint8_t *oBuff[3];
	const uint8_t *iBuff[3];
	int	strideTab[3];
	int	strideTab2[3];
	aviInfo _info;

        getVideoInfo(&_info);
        if(refOnly)
        {
                iBuff[0]= tmpImage->_planes[0];
                iBuff[1]= tmpImage->_planes[1];
                iBuff[2]= tmpImage->_planes[2];

                strideTab2[0]=_info.width;
                strideTab2[1]=_info.width>>1;
                strideTab2[2]=_info.width>>1;

                strideTab[0]=tmpImage->_planeStride[0];
                strideTab[1]=tmpImage->_planeStride[1];
                strideTab[2]=tmpImage->_planeStride[2];

        }
        else
        {
                iBuff[0]= YPLANE((tmpImage));
                iBuff[1]= UPLANE((tmpImage));
                iBuff[2]= VPLANE((tmpImage));

                strideTab[0]=strideTab2[0]=_info.width;
                strideTab[1]=strideTab2[1]=_info.width>>1;
                strideTab[2]=strideTab2[2]=_info.width>>1;
        }
        if(_pp.swapuv)
        {
                oBuff[0]= YPLANE(out);
                oBuff[1]= VPLANE(out);
                oBuff[2]= UPLANE(out);
        }else
        {

                oBuff[0]= YPLANE(out);
                oBuff[1]= UPLANE(out);
                oBuff[2]= VPLANE(out);
        }
        pp_postprocess(
            iBuff,
            strideTab,
            oBuff,
            strideTab2,
            ww,
            _info.height,
            (int8_t *)(tmpImage->quant),
            tmpImage->_qStride,
            _pp.ppMode,
            _pp.ppContext,
            type);			// img type
        /*
                If there is a chroma block that needs padding
                (width not multiple of 16) while postprocessing,
                we process up to the nearest 16 multiple and
                just copy luma & chroma info that was left over
        */
        if(refOnly && left)
        {
                uint8_t *src,*dst;
                uint32_t stridein,strideout,right;
                right=_info.width-left;
                // Luma
                dst=YPLANE(out)+right;
                src=tmpImage->_planes[0]+right;
                stridein=tmpImage->_planeStride[0];
                strideout=_info.width;
                for(uint32_t y=_info.height;y>0;y--)
                {
                        memcpy(dst,src,left);
                        dst+=strideout;
                        src+=stridein;
                }
                // Chroma
                left>>=1;
                right>>=1;
                //
                dst=UPLANE(out)+right;
                src=tmpImage->_planes[1]+right;
                stridein=tmpImage->_planeStride[1];
                strideout=_info.width>>1;
                for(uint32_t y=_info.height>>1;y>0;y--)
                {
                        memcpy(dst,src,left);
                        dst+=strideout;
                        src+=stridein;
                }
                //
                dst=VPLANE(out)+right;
                src=tmpImage->_planes[2]+right;
                stridein=tmpImage->_planeStride[2];
                strideout=_info.width>>1;
                for(uint32_t y=_info.height>>1;y>0;y--)
                {
                        memcpy(dst,src,left);
                        dst+=strideout;
                        src+=stridein;
                }


        }
    return true;
}

/**_____________________________________________________________________
		Main function
		Caller ask for a frame from the editor
		We first split it to segment and frame from that segment
			then look if it is in the cache
		If not and if there is Bframe we look if it is the forward
			reference frame we currently have

_______________________________________________________________________*/
uint8_t  ADM_Composer::getUncompressedFrame (uint32_t frame, ADMImage * out,
				      uint32_t * flagz)
{
  // first get segment
  uint32_t relframe, seg, flags, len;
  uint8_t ret = 0;
  EditorCache   *cache;
  ADMImage	*result;

//    static uint32_t lastlen=0;
    printf("[getUncompressedFrame] *************** OBSOLETE FUNCTION CALLED **********************\n");
    printf("[getUncompressedFrame] *************** OBSOLETE FUNCTION CALLED **********************\n");
    printf("[getUncompressedFrame] *************** OBSOLETE FUNCTION CALLED **********************\n");
    printf("[getUncompressedFrame] *************** OBSOLETE FUNCTION CALLED **********************\n");
    printf("[getUncompressedFrame] *************** OBSOLETE FUNCTION CALLED **********************\n");
    printf("[getUncompressedFrame] *************** OBSOLETE FUNCTION CALLED **********************\n");
	if(flagz)
			*flagz=0;

  // convert frame to block, relative frame
  if (!convFrame2Seg (frame, &seg, &relframe))
    {
      printf ("\n Conversion failed !\n");
      return 0;
    }
  uint32_t ref = _segments[seg]._reference;
  uint32_t llen = _videos[ref]._aviheader->getWidth ()
  				  * _videos[ref]._aviheader->getHeight ();
	llen = llen + (llen >> 1);
	_VIDEOS *vid=&_videos[ref];
	cache=_videos[ref]._videoCache;
	ADM_assert(cache);

	aprintf("Ed: Request for frame %"LU" seg %"LU", old frame:%"LU" old seg:%"LU"\n",relframe,seg,_lastframe,_lastseg);

	// First look in the cache

	if((result=cache->getImage(relframe)))
	{
		aprintf(">>frame %"LU" is cached...\n",relframe);
		out->duplicate(result);
		if(flagz)
			*flagz=result->flags;
		return 1;
	}
	else
	{
		aprintf("frame %"LU" is not cached...\n",relframe);
	}
//

//	Prepare the destination...
	result=cache->getFreeImage();
#ifdef VERBOSE
	cache->dump();
#endif

  // now we got segment and frame
  //*************************
  // Is is a key frame ?
  //*************************
  _videos[ref]._aviheader->getFlags (relframe, &flags);

    if (flags & AVI_KEY_FRAME)
    {
    	aprintf("keyframe\n");     	
        if(!decodeCache(relframe,ref, result))
		{
			printf("[edRender]Editor: Cannot deccode keyframe %"LU"\n",relframe);
			return 0;
		}
        _lastseg = seg;
        _lastframe = relframe;
        if(flagz)
            *flagz=result->flags;
        out->duplicate(result);
        return (1);
    }

    //*************************
    // following frame ?
    // If it a  next b-frame we already have the forward reference set
    // if it is a first B-frame --> do it
    //
    //*************************
      if ((seg == _lastseg) && ((_lastframe + 1) == relframe))
	{
	aprintf("following frame\n");
	// B Frame ?
#if 0
	if(_videos[ref]._aviheader->isReordered())
	{
		// The frame we seek is not in cache, so
		if(!_videos[ref]._aviheader->getFlags (relframe, &flags))
		{
			printf("Editor : Getflags failed\n");
			return 0;
		}
		// if it is a b frame we decode all of them up to
		// the next ip included and put all this in cache
		if(flags & AVI_B_FRAME)
		{
			// decode all of them up to the next I/P frame
			uint32_t nextIp=relframe;

			while((flags&AVI_B_FRAME))
			{
				nextIp++;
				_videos[ref]._aviheader->getFlags (nextIp, &flags);

			}
			// Decode it
			if(!decodeCache(nextIp,ref, result))
			{
				printf("Editor: Cannot read ip frame %"LU"\n",nextIp);
				return 0;
			}


			// And now go forward...
			uint32_t seeked=relframe;
			while(seeked<nextIp)
			{
				result=cache->getFreeImage();
				if(!decodeCache(seeked,ref, result))
				{
					printf("Editor: Cannot read ip frame %"LU"\n",nextIp);
					return 0;
				}
				if(seeked==relframe)
				{
					 out->duplicate(result);
					 if(flagz) *flagz=result->flags;
				}
				seeked++;
			}
			_lastframe=nextIp;
			_lastseg = seg;
			return 1;
		}

	}
#endif
	// No b frame...

      	 if(!decodeCache(relframe,ref, result))
			{
				printf("Editor: Cannot read ip frame %"LU"\n",relframe);
				return 0;
			}
	if(flagz)
		*flagz=result->flags;
	out->duplicate(result);
	_lastframe=relframe;
	_lastseg = seg;
	return 1;
    }
  //*************************
  // completly async frame
  // rewind ?
  //*************************

  aprintf("async  frame, wanted : %"LU" last %"LU" (%"LU" - %"LU" seg)\n",relframe,_lastframe,seg,_lastseg);
  uint32_t rewind;
  uint32_t seekFlag=0;

  _videos[ref]._aviheader->getFlags (relframe, &seekFlag);

  flags = 0;
  uint32_t need_rewind=1;
  rewind = relframe;
  ADM_assert(rewind); // the first frame should be a keyframe !
  while (!(flags & AVI_KEY_FRAME))
  {
  	rewind--;
  	_videos[ref]._aviheader->getFlags (rewind, &flags);
   }
   // Optimize for resample FPS*************************
   // If we are in the same segment, look if it is better to decode
    // From where we are or to seek the previous intra
   if ((seg == _lastseg))
   {
     if(rewind<_lastframe && relframe>_lastframe) // we have a better position to go from
     {
       for (uint32_t i = _lastframe+1; i <relframe; i++)
       {

         _videos[ref]._aviheader->getFlags (i, &flags);
      // Skip B frames, there can be a lot of them
         if((flags&AVI_B_FRAME)) continue;

         if(!decodeCache(i,ref, result))
         {
           printf("Editor: Cannot read ip frame %"LU"\n",relframe);
           return 0;
         }
         result=cache->getFreeImage();
       }
       need_rewind=0;
     }
   }
   // Optimize for resample FPS*************************
   //
  // now forward
  // IP seen is the last P (or I) before the frame we seek
   if(need_rewind)
   {
        for (uint32_t i = rewind; i <relframe; i++)
        {

                _videos[ref]._aviheader->getFlags (i, &flags);
                // Skip B frames, there can be a lot of them
                if((flags&AVI_B_FRAME)) continue;

                if(!decodeCache(i,ref, result))
	        {
		      printf("Editor: Cannot read ip frame %"LU"\n",relframe);
		      return 0;
	        }
	        result=cache->getFreeImage();
        }
   }
      // Time to decode our frame
      // if it is not a B, just decode it
      // it it is a B, the usual stuff
      if(!seekFlag)
      {
      		if(!decodeCache(relframe,ref, result))
			{
				printf("Editor: Cannot read ip frame %"LU"\n",relframe);
				return 0;
			}
		if(flagz)
			*flagz=result->flags;
		out->duplicate(result);
		_lastframe=relframe;
		_lastseg=seg;
		return 1;

      }
      // it is a b frame
      // decode all of them up to the next I/P frame
	uint32_t nextIp=relframe;
	flags=AVI_B_FRAME;
	while((flags&AVI_B_FRAME))
	{
		nextIp++;
		_videos[ref]._aviheader->getFlags (nextIp, &flags);
	}
	// Decode it
	if(!decodeCache(nextIp,ref, result))
	{
		printf("Editor: Cannot read ip frame %"LU"\n",nextIp);
		return 0;
	}


	// And now go forward...
	uint32_t seeked=relframe;
	while(seeked<nextIp)
	{
		result=cache->getFreeImage();
		if(!decodeCache(seeked,ref, result))
		{
			printf("Editor: Cannot read ip frame %"LU"\n",nextIp);
			return 0;
		}
		if(seeked==relframe)
		{
			 out->duplicate(result);
			 if(flagz) *flagz=result->flags;
		}
		seeked++;
	}
	_lastframe=nextIp;
	_lastseg=seg;
  	return 1;
}


/**
    \fn decodeCache
    \brief Decode an image an update cache

*/

uint8_t		ADM_Composer::decodeCache(uint32_t frame,uint32_t seg, ADMImage *image)
{

uint32_t sumit;
float	 sum;
EditorCache *cache=_videos[seg]._videoCache;
ADMImage *tmpImage=NULL;
uint8_t refOnly=0;
uint32_t left,ww;
ADMCompressedImage img;

        aprintf("decodeCache : Frame %u\n",frame);
        img.data=compBuffer;
        img.cleanup(frame);
    // Step 1, retrieve the compressed datas, including PTS & DTS infos
	 if (!_videos[seg]._aviheader->getFrame (frame,&img))
	{
	  printf ("\nEditor: last decoding failed.%"LD")\n",   frame );
	  return 0;
	}
        ADM_assert(_imageBuffer);

        // if len is 0 then take the previous image
        //

        refOnly=_videos[seg].decoder->dontcopy(); // can we skip one memcpy ?

        if(!img.dataLength & refOnly & !frame)      // Size is null = no image and we only got a pointer
                                // copy the previous one
        {
                // First image

                uint32_t page=_imageBuffer->_width*_imageBuffer->_height;
                        memset(YPLANE(image),0,page);
                        memset(UPLANE(image),128,page>>2);
                        memset(VPLANE(image),128,page>>2);
                        if(!frame)
                                image->flags=AVI_KEY_FRAME;
                        else
                                image->flags=AVI_P_FRAME;
                        image->_Qp=2;
                        image->_qStride=0;
                        return 1;
        }

        if(refOnly)
        {       // This is only an empty Shell
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
                  _scratch=new ADMImage(_imageBuffer->_width,_imageBuffer->_height,1);
                }
                tmpImage=_scratch;
                ww=_imageBuffer->_width & 0xfffff0;
                left=_imageBuffer->_width & 0xf;

        }
        else
        {
                tmpImage=_imageBuffer;
                ww=_imageBuffer->_width;
                left=0;
       }
	tmpImage->_colorspace=ADM_COLOR_YV12;
	// Do pp, and use imageBuffer as intermediate buffer
	if (!_videos[seg].decoder->uncompress (&img, tmpImage))
	    {
	      printf ("\nEditor: Last Decoding2 failed for frame %"LU"\n",frame);
	       // Try to dupe previous frame
                if(frame)
                {
                        ADMImage *prev;
                        prev=cache->getImage(frame-1);
                        if(prev)
                        {
                                image->duplicate(prev);
                                cache->updateFrameNum(image,frame);
                                return 1;
                        }
                }
                goto _next;
           }

        //
        if(tmpImage->_noPicture && refOnly && frame)
        {
                cache->updateFrameNum(image,0xfffffffffLL);
                return 0;
        }
	// If not quant and it is already YV12, we can stop here
	if((!tmpImage->quant || !tmpImage->_qStride) && tmpImage->_colorspace==ADM_COLOR_YV12)
	{
		image->_Qp=2;
		image->duplicate(tmpImage);
		cache->updateFrameNum(image,frame);
          //      if(refOnly) delete tmpImage;
		aprintf("EdCache: No quant avail\n");
		return 1;
	}
	// We got everything, let's go
	// 1 compute average quant
	int qz;
	sumit=0;
        // Dupe infos
        image->copyInfo(tmpImage);


        // Do postprocessing if any
	for(uint32_t z=0;z<tmpImage->_qSize;z++)
	{
            qz=(int)tmpImage->quant[z];
			sumit+=qz;
	}
	sumit+=(tmpImage->_qSize-1);
//	sumit*=2;
	sum=(float)sumit;
	sum/=tmpImage->_qSize;
	if(sum>31) sum=31;
	if(sum<1) sum=1;

        // update average Q
	tmpImage->_Qp=image->_Qp=(uint32_t)floor(sum);

	// Pp deactivated ?
	if(!_pp.postProcType || !_pp.postProcStrength || tmpImage->_colorspace!=ADM_COLOR_YV12)
         {
                dupe(tmpImage,image,&(_videos[seg]));
		cache->updateFrameNum(image,frame);
               // if(refOnly) delete tmpImage;
		aprintf("EdCache: Postproc disabled\n");
		return 1;
	}

	int type;
	#warning FIXME should be FF_I_TYPE/B/P
	if(tmpImage->flags & AVI_KEY_FRAME) type=1;
		else if(tmpImage->flags & AVI_B_FRAME) type=3;
			else type=2;

        ADM_assert(tmpImage->_colorspace==ADM_COLOR_YV12);

	// we do postproc !
	// keep
	uint8_t *oBuff[3];
	const uint8_t *iBuff[3];
	int	strideTab[3];
	int	strideTab2[3];
	aviInfo _info;

		getVideoInfo(&_info);
                if(refOnly)
                {
                        iBuff[0]= tmpImage->_planes[0];
                        iBuff[1]= tmpImage->_planes[1];
                        iBuff[2]= tmpImage->_planes[2];

                        strideTab2[0]=_info.width;
                        strideTab2[1]=_info.width>>1;
                        strideTab2[2]=_info.width>>1;

                        strideTab[0]=tmpImage->_planeStride[0];
                        strideTab[1]=tmpImage->_planeStride[1];
                        strideTab[2]=tmpImage->_planeStride[2];

                }
                else
                {
		        iBuff[0]= YPLANE((tmpImage));
                iBuff[1]= UPLANE((tmpImage));
                iBuff[2]= VPLANE((tmpImage));



                        strideTab[0]=strideTab2[0]=_info.width;
                        strideTab[1]=strideTab2[1]=_info.width>>1;
                        strideTab[2]=strideTab2[2]=_info.width>>1;
                }
                if(_pp.swapuv)
                {
        	        oBuff[0]= YPLANE(image);
                        oBuff[1]= VPLANE(image);
                        oBuff[2]= UPLANE(image);
                }else
                {

                        oBuff[0]= YPLANE(image);
                        oBuff[1]= UPLANE(image);
                        oBuff[2]= VPLANE(image);
                }
		 pp_postprocess(
		 		iBuff,
		 		strideTab,
		 		oBuff,
		 		strideTab2,
		 		ww,
		        	_info.height,
		          	(int8_t *)(tmpImage->quant),
		          	tmpImage->_qStride,
		         	_pp.ppMode,
		          	_pp.ppContext,
		          	type);			// img type
                /*
                        If there is a chroma block that needs padding
                        (width not multiple of 16) while postprocessing,
                        we process up to the nearest 16 multiple and
                        just copy luma & chroma info that was left over
                */
                if(refOnly && left)
                {
                        uint8_t *src,*dst;
                        uint32_t stridein,strideout,right;
                        right=_info.width-left;
                        // Luma
                        dst=YPLANE(image)+right;
                        src=tmpImage->_planes[0]+right;
                        stridein=tmpImage->_planeStride[0];
                        strideout=_info.width;
                        for(uint32_t y=_info.height;y>0;y--)
                        {
                                memcpy(dst,src,left);
                                dst+=strideout;
                                src+=stridein;
                        }
                        // Chroma
                        left>>=1;
                        right>>=1;
                        //
                        dst=UPLANE(image)+right;
                        src=tmpImage->_planes[1]+right;
                        stridein=tmpImage->_planeStride[1];
                        strideout=_info.width>>1;
                        for(uint32_t y=_info.height>>1;y>0;y--)
                        {
                                memcpy(dst,src,left);
                                dst+=strideout;
                                src+=stridein;
                        }
                        //
                        dst=VPLANE(image)+right;
                        src=tmpImage->_planes[2]+right;
                        stridein=tmpImage->_planeStride[2];
                        strideout=_info.width>>1;
                        for(uint32_t y=_info.height>>1;y>0;y--)
                        {
                                memcpy(dst,src,left);
                                dst+=strideout;
                                src+=stridein;
                        }


                }
_next:
        // update some infos
        //   if(refOnly) delete tmpImage;
		cache->updateFrameNum(image,frame);
		aprintf("EdCache: Postproc done\n");
		return 1;
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
	if(!_nb_video) return 0;
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
	if(!_nb_video) return 0;
	*type=_pp.postProcType;
	*strength=_pp.postProcStrength;
	*swapuv=_pp.swapuv;
	return 1;
}
//______________________________________________
//_______________________________________________
/**
    \fn getPKFrame
    \brief returns the keyFrame strictly before *frame
*/
uint8_t	ADM_Composer::getPKFrame(uint32_t *frame)
{
	uint32_t fr, seg, relframe;	//,len; //flags,ret,nf;

  fr = *frame;

  if (*frame == 0)
    {
      return 0;
    }
  if (!searchPreviousKeyFrame (fr, &seg, &relframe))
    {
      printf (" NKF not found\n");
      return 0;
    }
  ADM_assert (convSeg2Frame (frame, seg, relframe));
  return 1;
}
/**
    \fn getNKFrame
    \brief returns the keyFrame strictly after *frame
*/

uint8_t	ADM_Composer::getNKFrame(uint32_t *frame)
{
	uint32_t fr, seg, relframe;	//,len; //flags,ret,nf;

  fr = *frame;
  if (!searchNextKeyFrame (fr, &seg, &relframe))
    {
      printf (" NKF not found\n");
      return 0;
    }
  ADM_assert (convSeg2Frame (frame, seg, relframe));
  return 1;
}

#if 0
uint8_t	ADM_Composer::isReordered( uint32_t framenum )
{
uint32_t seg,relframe;
 if (!convFrame2Seg (framenum, &seg, &relframe))
    {
      printf ("\n Conversion failed !\n");
      return 0;
    }
    uint32_t ref=_segments[seg]._reference;
   return _videos[ref]._aviheader->isReordered();
}
#endif
/**
    \fn getCurrentFrame
*/
uint32_t    ADM_Composer::getCurrentFrame(void)
{
    return currentFrame;
}
/**
    \fn setCurrentFrame
*/
bool        ADM_Composer::setCurrentFrame(uint32_t frame)
{
    // Seatch previous keyFrame
    printf("[setCurrentFrame] >>> REQ for frame %"LU"\n",frame);
    uint32_t keyFrame=frame,f;
    while(keyFrame)
    {
        if(getFlags(keyFrame,&f))
        {
            if(f&AVI_KEY_FRAME) break;
        }
        keyFrame--;
    }
    printf("[setCurrentFrame] >>> Prev KeyFrame %"LU"\n",keyFrame);
    if(false==GoToIntra(keyFrame))
    {
        printf("[setCurrentFrame] GoToIntra failed for frame %u\n",keyFrame);
        return false;
    }
    //Now go forward
    uint32_t mx=frame-keyFrame;
    uint32_t i=0;
    for(i=0;i<mx;i++)
    {
        if(NextPicture(NULL) ==false)
        {
            return false;
        }
    }
    return true;
}
/**
    \fn searchFrameBefore
    \brief rEturn the frame number with pts just before pts
*/
uint32_t ADM_Composer::searchFrameBefore(uint64_t pts)
{
    _VIDEOS   *vid=&_videos[0];
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
//EOF
