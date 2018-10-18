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

#if 1
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
    aviInfo info;
    video_body->getVideoInfo(&info);
    if(isH264Compatible( info.fcc)) h265=false; //=4;
    else if(isH265Compatible( info.fcc)) h265=true; //=5;
    else ADM_assert(0);
}

/**
    \fn ADM_videoStreamCopyAudRemover
*/
ADM_videoStreamCopyAudRemover::~ADM_videoStreamCopyAudRemover()
{

}

/**
    \fn getPacket
*/
bool  ADM_videoStreamCopyAudRemover::getPacket(ADMBitstream *out)
{
    if(!ADM_videoStreamCopy::getPacket(out)) 
        return false;
    //return true;
    // Remove AUDs in place
    NALU_descriptor desc[51];
    int size=4;
    if(h265) size=5;
    
    int nbNalu=ADM_splitNalu_internal(out->data, out->data+out->len,50,desc,size);
    if(!nbNalu) return true;
    
    uint8_t *head=out->data;
    for(int i=0;i<nbNalu;i++)
    {
        bool copy=true;
        if(!h265 && desc[i].nalu==NAL_AU_DELIMITER) copy=false;
        if(h265 && desc[i].nalu==NAL_H265_AUD) copy=false;
        
        if(copy==false) continue;
        //
        if(head==desc[i].start) // nothing to do, already at the right place
        {
            head+=desc[i].size+size;
            continue;
        }
        // Else copy
        memmove(head,desc[i].start-size,desc[i].size+size); // also copy NAL header
        head+=desc[i].size+size;
    }
    out->len=(intptr_t)head-(intptr_t)out->data;
    return true;
}

// EOF

