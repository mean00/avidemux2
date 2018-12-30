/**
    \file ADM_videoCopy
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
using std::string;
#include "ADM_default.h"
#include "ADM_videoCopy.h"
#include "ADM_edit.hxx"
#include "ADM_coreUtils.h"
#include "ADM_vidMisc.h"
#include "prefs.h"
#include "ADM_videoInfoExtractor.h"
#include "ADM_h264_tag.h"
#include "ADM_h265_tag.h"
extern ADM_Composer *video_body; // Fixme!

#if 0
#define aprintf ADM_info
#else
#define aprintf(...) {}
#endif

/**
    \fn ADM_videoStreamCopyAudRemover
*/
ADM_videoStreamCopyAudRemover::ADM_videoStreamCopyAudRemover(uint64_t startTime,uint64_t endTime) : ADM_videoStreamCopy(startTime,endTime)
{
    ADM_info("AUD Remover created\n");
    carryover=0;
    aviInfo info;
    video_body->getVideoInfo(&info);
    if(isH264Compatible( info.fcc)) h265=false;
    else if(isH265Compatible( info.fcc)) h265=true;
    else ADM_assert(0);
}

/**
    \fn ADM_videoStreamCopyAudRemover
*/
ADM_videoStreamCopyAudRemover::~ADM_videoStreamCopyAudRemover()
{

}

static bool needsLongStartCodePrefix(int nalu, bool hevc)
{
    int type;
    if(hevc)
        type=(nalu>>1)&0x3f;
    else
        type=nalu&0x1f;

    if(hevc)
    {
        switch(type)
        {
            case NAL_H265_VPS:
            case NAL_H265_SPS:
            case NAL_H265_PPS:
            case NAL_H265_AUD:
                return true;
            default:
                return false;
        }
    }

    switch(type)
    {
        case NAL_SPS:
        case NAL_PPS:
        case NAL_AU_DELIMITER:
            return true;
        default:
            return false;
    }
}

/**
    \fn getPacket
*/
bool  ADM_videoStreamCopyAudRemover::getPacket(ADMBitstream *out)
{
    if(!ADM_videoStreamCopy::getPacket(out)) 
        return false;
    // Remove AUDs in place
    static NALU_descriptor desc[51]; // Only one instance max, no risk of simulatenous access
    uint8_t *head=out->data;
    bool removeLeadingAud=false;
    // If the current packet is a keyframe, we might be at a cut,
    // the stuff from the previous packet doesn't belong here.
    // Else copy it to the start of the data.
    if(carryover && !(out->flags & AVI_KEY_FRAME))
    {
        if(out->len+carryover>out->bufferSize) // no space
        {
            ADM_warning("Buffer size insufficient to reconstruct frame!\n");
            carryover=0;
        }else
        {
            memmove(head+carryover,head,out->len);
            memcpy(head,scratchpad,carryover);
            head+=carryover;
            carryover=0;
            removeLeadingAud=true;
        }
    }

    int nbNalu=ADM_splitNalu(head,head+out->len,50,desc);
    if(!nbNalu) return true;

    bool copyToScratchpad=false;
    int nbAddedZeroBytes=0;

    for(int i=0;i<nbNalu;i++)
    {
        NALU_descriptor *d=desc+i;
        d->start+=nbAddedZeroBytes;
        bool needsZeroByte=needsLongStartCodePrefix(d->nalu,h265);
        int size=4+d->zerobyte;
#if 0
        printf("NALU %d: type %d, start %p (head at %p), prefix len %d, payload size %d\n",i,
                h265? ((d->nalu>>1)&0x3f) : (d->nalu&0x1f),
                d->start,head,size,d->size);
#endif
        if(h265)
        {
            if(((d->nalu>>1)&0x3f)==NAL_H265_AUD)
            {
                if(i)
                {
                    if(copyToScratchpad) break; // We've already got what we wanted, stop here.
                    // NALUs starting with this AUD up to the next one belong to the next frame.
                    copyToScratchpad=true;
                }else if(removeLeadingAud)
                {
                    removeLeadingAud=false;
                    continue;
                }
            }
        }else
        {
            if(i && (d->nalu&0x1f)==NAL_AU_DELIMITER) break;
        }

        if(copyToScratchpad)
        {
            if(!d->zerobyte && needsZeroByte)
            {
                memset(scratchpad+carryover,0,1);
                carryover++;
            }
            memcpy(scratchpad+carryover,d->start-size,d->size+size);
            carryover+=d->size+size;
            continue;
        }

        if(head==d->start-size) // nothing to do, already at the right place
        {
            if(d->zerobyte || !needsZeroByte)
            {
                head+=d->size+size;
                continue;
            }
            uint64_t filled=(intptr_t)head-(intptr_t)out->data;
            memmove(head+1,d->start-size,out->len+nbAddedZeroBytes-filled);
            memset(head,0,1);
            nbAddedZeroBytes++;
            head+=d->size+5;
            continue;
        }
        // Else copy
        memmove(head,d->start-size,d->size+size); // also copy NAL header
        head+=d->size+size;
    }
    uint64_t org=out->len;
    out->len=(intptr_t)head-(intptr_t)out->data;
    
    if(out->len<org)
        aprintf("Saved %d bytes\n",(int)(org-out->len));
    if(out->len>org)
        aprintf("Added %d bytes\n",(int)(out->len-org));

    return true;
}

// EOF

