/**
    \file ADM_videoCopySeiInjector
    \brief Wrapper
    (c) Mean 2008/GPLv2
*/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "ADM_cpp.h"
#include "ADM_default.h"
#include "ADM_videoCopy.h"
#include "ADM_edit.hxx"
#include "ADM_coreUtils.h"
#include "ADM_videoInfoExtractor.h"
#include "ADM_h264_tag.h"
extern ADM_Composer *video_body; // Fixme!

/**
 *  \fn ctor
 */
ADM_videoStreamCopySeiInjector::ADM_videoStreamCopySeiInjector(uint64_t startTime,uint64_t endTime) : ADM_videoStreamCopy(startTime,endTime)
{
    ADM_info("SEI injector created\n");
    uint32_t length=0;
    seiLen=0;
    nalSize=0;
    {
        uint8_t *extra;
        uint32_t extraLen=0;
        getExtraData(&extraLen,&extra);
        if(extraLen)
            nalSize=ADM_getNalSizeH264(extra,extraLen);
        if(!nalSize)
        {
            ADM_warning("Cannot get valid extradata, passthrough operation.\n");
            return;
        }
    }
    if(video_body->getUserDataUnregistered(startTime, seiBuf, ADM_H264_MAX_SEI_LENGTH, &length))
    {
        ADM_info("SEI message with x264 build info present, NAL unit length: %u\n",length);
        mixDump(seiBuf,length);
        seiLen=length;
    }else
    {
        ADM_info("No x264 build info found, purely passthrough operation.\n");
    }
}

/**
 *  \fn dtor
 */
ADM_videoStreamCopySeiInjector::~ADM_videoStreamCopySeiInjector()
{

}

/**
 *  \fn getPacket
 */
bool  ADM_videoStreamCopySeiInjector::getPacket(ADMBitstream *out)
{
    if(!ADM_videoStreamCopy::getPacket(out))
        return false;
    if(!seiLen)
        return true;
    if(!(out->flags & AVI_KEY_FRAME))
        return true;
    // Check whether this particular SEI message is already present.
    uint32_t len=0;
    if(extractH264SEI(out->data, out->len, nalSize, NULL, ADM_H264_MAX_SEI_LENGTH, &len))
    {
        if(len!=seiLen)
            ADM_warning("SEI message present, but the length is different?\n");
        else
            ADM_info("SEI message with x264 version info already present, nothing to do.\n");
        seiLen=0;
        return true;
    }
    // Nope, do we have enough space to inject ours?
    if(out->len+seiLen>=out->bufferSize)
    {
        ADM_warning("Not enough free buffer to inject SEI.\n"); // we'll try with the next keyframe
        return true;
    }
    // Inject SEI from seiBuf
    uint8_t *tail=out->data;
    uint8_t *head=tail+out->len;
    uint8_t stream;

    while(tail + nalSize < head)
    {
        uint32_t i,length=0;
        for(i=0; i<nalSize; i++)
            length=(length<<8)+tail[i];
        stream=*(tail+nalSize)&0x1F;
        switch(stream)
        {
            case NAL_SEI:
            case NAL_IDR:
            case NAL_NON_IDR:
            {
                memmove(tail+seiLen,tail,head-tail);
                memcpy(tail,seiBuf,seiLen);
                out->len+=seiLen;
                seiLen=0;
                ADM_info("SEI message with x264 version info injected.\n");
                return true;
            }
            default:
                tail+=nalSize+length;
                continue;
        }
    }

    return true;
}

// EOF

