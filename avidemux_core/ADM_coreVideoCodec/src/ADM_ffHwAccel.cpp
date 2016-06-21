/***************************************************************************
    \file ADM_ffHwAccel
    \brief Decoders using lavcodec
    \author mean & all (c) 2002-2010
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <stddef.h>

#include "ADM_default.h"
#include "ADM_ffmp43.h"
#include "DIA_coreToolkit.h"
#include "ADM_hwAccel.h"
#include <vector>

static std::vector<ADM_hwAccelEntry *>listOfHwAccel;

/**
 * \fn registerDecoder
 */
bool                ADM_hwAccelManager::registerDecoder(ADM_hwAccelEntry *entry)
{
    listOfHwAccel.push_back(entry);
    return false;
}
/**
 * \fn lookup
 */
ADM_hwAccelEntry    *ADM_hwAccelManager::lookup(struct AVCodecContext *avctx,  const enum AVPixelFormat *fmt,enum AVPixelFormat &outputFormat)
{
    int n=listOfHwAccel.size();
    for( int i=0;i<n;i++)
    {
        ADM_hwAccelEntry *e=listOfHwAccel[i];
        if(true==e->canSupportThis(avctx,fmt,outputFormat))
        {
            ADM_info("Matching hw accel : %s\n",e->name);
            return e;
        }
    }
    ADM_info("No Matching Hw accel\n");
    return NULL;;
}

/**
 * 
 * @param avctx
 * @param fmt
 * @return 
 */
extern enum AVPixelFormat ADM_FFgetFormat(struct AVCodecContext *avctx,  const enum AVPixelFormat *fmt)
{
    enum AVPixelFormat outFmt;
    ADM_hwAccelEntry    *accel=ADM_hwAccelManager::lookup(avctx,fmt,outFmt);
    if(accel)
    {
        decoderFF *ff=(decoderFF *)avctx->opaque;
        if(ff->getHwDecoder())
        {
            ADM_info("Reusing existing setup\n");        
            return outFmt;
        }else
            if(ff->setHwDecoder(accel->spawn(avctx,fmt)))
            {
                ADM_info("Using %s as hw accel\n",accel->name);
                return outFmt;
            }
    }
    ADM_info("No Hw Accel for that\n");
    return avcodec_default_get_format(avctx,fmt);
}


/**
 * 
 * @param pic
 * @return 
 */
uint32_t ADM_acceleratedDecoderFF::admFrameTypeFromLav (AVFrame *pic)
{
    
#define SET(x)      {outFlags=x;}
#define SET_ADD(x)  {outFlags|=x;}
  uint32_t outFlags=0;
  switch (pic->pict_type)
    {
        case AV_PICTURE_TYPE_B:
                SET (AVI_B_FRAME);
                break;
        case AV_PICTURE_TYPE_I:
                SET (AVI_KEY_FRAME);
                if (!pic->key_frame)
                  {
                    if (_context->codec_id == AV_CODEC_ID_H264)
                        SET (AVI_P_FRAME)
                    else
                      ADM_info ("\n But keyframe is not set\n");
                  }
                break;
        case AV_PICTURE_TYPE_S:
        case AV_PICTURE_TYPE_P:
                SET (AVI_P_FRAME);
                break;
        default:
                break;
    }
    outFlags&=~AVI_STRUCTURE_TYPE_MASK;
    if(pic->interlaced_frame)
    {
        SET_ADD(AVI_FIELD_STRUCTURE)
        if(pic->top_field_first)
            SET_ADD(AVI_TOP_FIELD)
        else
            SET_ADD(AVI_BOTTOM_FIELD)
    }
  return outFlags;
}
/**
 * 
 * @param pix_fmt
 * @param id
 * @return 
 */
const AVHWAccel *ADM_acceleratedDecoderFF::parseHwAccel(enum AVPixelFormat pix_fmt,AVCodecID id,AVPixelFormat searchedItem)
{
    AVHWAccel *hw=av_hwaccel_next(NULL);
    
    while(hw)
    {
        ADM_info("Trying %s, hwPixFmt=%d, wantedPixFmt %d, hwCodecId =%d : wantedCodecID=%d\n",hw->name,hw->pix_fmt,pix_fmt,hw->id,id);
        if (hw->pix_fmt == searchedItem && id==hw->id)
            return hw;
        hw=av_hwaccel_next(hw);
    }
    return NULL;
}


// EOF