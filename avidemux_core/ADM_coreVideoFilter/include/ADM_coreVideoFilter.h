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

#include "ADM_coreVideoFilter6_export.h"
#include "BVector.h"
#include "ADM_confCouple.h"
#include "ADM_image.h"
#include "ADM_videoFilterCache.h"
#include "ADM_filterCategory.h"
#include "ADM_coreVideoFilterInternal.h"

/**
    \struct ADM_VideoFilterElement
*/
typedef struct
{
    uint32_t            tag; // Temporary filter tag
    ADM_coreVideoFilter *instance;
    int objectId;
}ADM_VideoFilterElement;

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
class ADM_COREVIDEOFILTER6_EXPORT ADM_coreVideoFilter
{
protected:
            FilterInfo            info;
            uint32_t             nextFrame; // next frame to fetch, it is reset to 0 after a seek!
            const char           *myName;
            
public:
                            ADM_coreVideoFilter(ADM_coreVideoFilter *previous,CONFcouple *conf=NULL);
       virtual             ~ADM_coreVideoFilter();

       virtual const char   *getConfiguration(void);                   /// Return  current configuration as a human readable string
       virtual bool         goToTime(uint64_t usSeek);                 /// Overide this if you have cleanup to do after a jump
       virtual bool         getNextFrame(uint32_t *frameNumber,ADMImage *image)=0;              /// Dont mix getFrame & getNextFrame !
       virtual bool         getNextFrameAs(ADM_HW_IMAGE type,uint32_t *frameNumber,ADMImage *image);              /// Request frame as type (hw accel)
       virtual FilterInfo  *getInfo(void);                             /// Return picture parameters after this filter
       virtual bool         getCoupledConf(CONFcouple **couples)=0 ;   /// Return the current filter configuration
       virtual void         setCoupledConf(CONFcouple *couples)=0;
       virtual bool         configure(void) {return true;}             /// Start graphical user interface
       virtual uint64_t     getAbsoluteStartTime(void)                 /// Return the absolute offset of the current frame. Used to display time of for filter
                {return previousFilter->getAbsoluteStartTime();}       /// Like subtitlers who need that
               ADM_coreVideoFilter *getSource() {return previousFilter;} /// FOR INTERNAL USE ONLY
protected:
            ADM_coreVideoFilter *previousFilter;
};
/**
 *  \class ADM_coreVideoFilterCached
 *  \brief Same as above but we use a cache. Beware of flushing the cash upon seeking!
 */
class ADM_COREVIDEOFILTER6_EXPORT ADM_coreVideoFilterCached : public ADM_coreVideoFilter
{
protected:
            VideoCache           *vidCache;
public:
                            ADM_coreVideoFilterCached(int cacheSize,ADM_coreVideoFilter *previous,CONFcouple *conf=NULL);
       virtual             ~ADM_coreVideoFilterCached();

       virtual bool         goToTime(uint64_t usSeek);                 /// Overide this if you have cleanup to do after a jump
};

// Avisynth compatibility functions
ADM_COREVIDEOFILTER6_EXPORT int PutHintingData(uint8_t *video, unsigned int hint);
ADM_COREVIDEOFILTER6_EXPORT int GetHintingData(uint8_t *video, unsigned int *hint);

extern ADM_COREVIDEOFILTER6_EXPORT BVector <ADM_VideoFilterElement> ADM_VideoFilters;
extern ADM_COREVIDEOFILTER6_EXPORT BVector <ADM_vf_plugin *> ADM_videoFilterPluginsList[VF_MAX];


#endif
// EOF
