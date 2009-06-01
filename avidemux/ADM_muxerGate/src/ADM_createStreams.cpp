/***************************************************************************
                          \fn ADM_createStreams.cpp
                          \brief Take a filterChain & create a stream with it
                           (c) Mean 2009



 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "ADM_default.h"
#include "ADM_coreVideoFilter.h"
#include "ADM_filterChain.h"
#include "ADM_coreVideoEncoder.h"
#include "ADM_muxer.h"
#include "ADM_videoProcess.h"

extern ADM_coreVideoEncoder *createVideoEncoderFromIndex(ADM_coreVideoFilter *chain,int index);
/**
    \fn createVideoStream
    \brief Create encoder then VideoStream from filterChain
    @return created VideoStream
*/
ADM_videoStream  *createVideoStream(ADM_videoFilterChain *chain,int index)
{
    int sz=chain->size();
    ADM_assert(sz);
    ADM_coreVideoFilter *filter=(*chain)[sz-1];
    ADM_assert(filter);
    ADM_coreVideoEncoder *encoder=createVideoEncoderFromIndex(filter,index);
    if(!encoder)
    {
        printf("[createVideoEncoder] Cannot create encoder\n");
        return NULL;
    }
    ADM_videoStreamProcess *stream=new ADM_videoStreamProcess(encoder);
    return stream;
}