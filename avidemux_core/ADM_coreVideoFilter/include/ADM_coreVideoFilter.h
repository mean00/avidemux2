/***************************************************************************
                          \fn ADM_coreVideoFilter.h
                          \brief Base class for Video Filters
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

#ifndef ADM_CORE_VIDEO_FILTER
#define  ADM_CORE_VIDEO_FILTER

#include "ADM_confCouple.h"
#include "ADM_image.h"
/**
    \struct FilterInfo
    \brief Describes the video stream at this point in the filter chain
*/
typedef struct
{
    uint32_t width;
    uint32_t height;
    uint32_t frameIncrement; /// Minimum Delta time between 2 frames in useconds ~ 1/fps
    uint64_t totalDuration;     /// Duration of the whole stream in us
}FilterInfo;

/**
 *  \class ADM_coreVideoFilter
 *  \brief base class for all video filters
 */
class ADM_coreVideoFilter
{
protected:
            FilterInfo           info;
            uint32_t             nextFrame;
public:
            ADM_coreVideoFilter(ADM_coreVideoFilter *previous,CONFcouple *conf=NULL);
            ~ADM_coreVideoFilter();

       virtual const char   *getConfiguration(void);                   /// Return  current configuration as a human readable string
       virtual bool         goToTime(uint64_t usSeek);              
       virtual bool         getNextFrame(ADMImage *image)=0;              /// Dont mix getFrame & getNextFrame !
	   virtual FilterInfo  *getInfo(void);                             /// Return picture parameters after this filter
	   virtual bool         getCoupledConf(CONFcouple **couples)=0 ;   /// Return the current filter configuration
       virtual bool         configure(void) {return true;}             /// Start graphical user interface
protected:
            ADM_coreVideoFilter *previousFilter;
};

#endif
// EOF