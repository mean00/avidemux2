//
// C++ Interface: ADM_cache
//
// Description: 
//
//
// Author: mean <fixounet@free.fr>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef __ADM_CACHE__
#define __ADM_CACHE__
class ADM_coreVideoFilter;

#include "ADM_coreVideoFilter6_export.h"
#include "ADM_image.h"
/**
    \struct videoCacheEntry
*/
typedef struct vidCacheEntry
{
                uint32_t 	frameNum;
                ADMImage 	*image;
                uint8_t		frameLock;		
                uint32_t	lastUse;
        bool        freeEntry;

}vidCacheEntry;
/**
    \class VideoCache
*/
class ADM_COREVIDEOFILTER6_EXPORT VideoCache
{
        private:
                vidCacheEntry       *entry;
                uint32_t            counter;
                uint32_t            nbEntry;
                ADM_coreVideoFilter *incoming;


                int32_t             searchFrame( uint32_t frame);
                int32_t             searchPtr( ADMImage *ptr);
                int                 searchFreeEntry(void);
                ADMImage            *getImageBase(uint32_t frame);
        public:
                                    VideoCache(uint32_t nb,ADM_coreVideoFilter *in);
                                    ~VideoCache(void);

                ADMImage            *getImage(uint32_t frame);
                ADMImage            *getImageAs(ADM_HW_IMAGE type,uint32_t frame);
                uint8_t             unlockAll(void);
                uint8_t             unlock(ADMImage  *frame);
                uint8_t             flush(void);
                void                dump(void);
};
#endif
