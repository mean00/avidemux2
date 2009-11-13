/***************************************************************************
                          \fn ADM_videoFilterBridge.h
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
#ifndef ADM_VIDEO_FILTER_BRIDGE_H
#define ADM_VIDEO_FILTER_BRIDGE_H

#include "ADM_coreVideoFilter.h"
/**
    \class ADM_videoFilterBridge
    \brief Class that transform Editor API to coreVideoFilter API

*/
class ADM_videoFilterBridge : public ADM_coreVideoFilter
{
protected:
        uint64_t            startTime,endTime;
        FilterInfo          bridgeInfo;
        bool                firstImage;
        uint32_t            lastSentImage;
        virtual bool        nextFrame(ADMImage *image);    /// Return the next image
public:
                            ADM_videoFilterBridge(uint64_t startTime, uint64_t endTime);
                            ~ADM_videoFilterBridge();
       virtual bool         getFrame(uint32_t frame,ADMImage *image);
       
	   virtual FilterInfo  *getInfo(void);                                      /// Return picture parameters after this filter
	   virtual bool         getCoupledConf(CONFcouple **couples) {*couples=NULL;return true;} ; /// Return the current filter configuration

        bool                rewind(void);

};

#endif
