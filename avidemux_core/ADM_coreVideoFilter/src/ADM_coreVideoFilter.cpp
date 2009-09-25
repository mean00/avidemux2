/***************************************************************************
                          \fn ADM_coreVideoFilter.h
                          \brief Base class for Video Filters
                           (c) Mean 2009

    This is used to build the video filter chain
    It is a simple "one in, one out" system based on pull.
    I.e. the final client asks for a frame and each stage asks its predecessor for a frame
if needed.

    The fist stage is the VideoFilterBridge. Its translates ADM_composer API to
    ADM_coreVideoFilter API.
    Then a bunch of ADM_coreVideoFilter using the ADM_coreVideoFilter API
    And at last the ADM_coreVideoEncoder that translate ADM_coreVideoFilter api
        to ADM_videoStream API ready to be sent to the muxer

    ADM_videoBody -> ADM_VideoFilterBridge > coreFilter->coreFilter.....->coreFilter->coreVideoEncoder


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
/**
    \fn ADM_coreVideoFilter

*/
 ADM_coreVideoFilter::ADM_coreVideoFilter(ADM_coreVideoFilter *previous,CONFcouple *conf)
{
    previousFilter=previous;
    nextFrame=0;
}
/**
    \fn  ~ADM_coreVideoFilter
*/
 ADM_coreVideoFilter:: ~ADM_coreVideoFilter()
{

}
/**
    \fn getConfiguration
*/
const char        *ADM_coreVideoFilter::getConfiguration(void)
{
    return "base";
}  
/**
    \fn getInfo
    \brief default behaviour, we return the Info as is from the previous filter in the chain
*/
FilterInfo  *ADM_coreVideoFilter::getInfo(void)
{
    ADM_assert(previousFilter);
    return previousFilter->getInfo();
}
/**
        \fn getNextFrame
        \brief 
*/  
bool         ADM_coreVideoFilter::getNextFrame(ADMImage *image)     
{
      bool r=getFrame(nextFrame,image);
      nextFrame++;
      return true;
}
// EOF
