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
#if 0
extern ADM_coreVideoEncoder *createVideoEncoderFromIndex(ADM_coreVideoFilter *chain,int index);
/**
    \fn createVideoStream
    \brief Create encoder then VideoStream from filterChain
    @return created VideoStream
*/
ADM_videoStream  *createVideoStream(ADM_coreVideoEncoder *encoder)
{
   
    ADM_videoStreamProcess *stream=new ADM_videoStreamProcess(encoder);
    return stream;
}

ADM_coreVideoFilter *getLastFilter(void)
{
    int sz=chain->size();
    ADM_assert(sz);
    ADM_coreVideoFilter *filter=(*chain)[sz-1];
    ADM_assert(filter);
    return filter;
}
#endif
//EOF
