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
    // Remove AUDs in place
    return true;
}

// EOF
