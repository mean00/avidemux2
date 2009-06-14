/***************************************************************************
                          Port of avisynth ColorYuv Filter
    copyright            : (C) 2006 by mean
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef ADM_VID_ANIMATED_H
#define ADM_VID_ANIMATED_H



#define LEFT_MARGIN 50
#define TOP_MARGIN 50

#include "ADM_vidAnimated_param.h"

class  ADMVideoAnimated:public AVDMGenericVideoStream
{

    protected:

        virtual     char            *printConf(void) ;
                    ANIMATED_PARAM  *_param;
                    ADMImageResizer *_resizer;
                    ADMImage        *_image;
                    ADMImage        *_BkgGnd;
                    VideoCache      *_caches[MAX_VIGNETTE];
                    uint8_t         setup( void);
                    uint8_t         cleanup( void);
                    uint8_t         loadImage(void);
    public:
                    ADMVideoAnimated(  AVDMGenericVideoStream *in,CONFcouple *setup);
                    ~ADMVideoAnimated();
        virtual uint8_t getFrameNumberNoAlloc(uint32_t frame, uint32_t *len,
                ADMImage *data,uint32_t *flags);

        virtual uint8_t configure( AVDMGenericVideoStream *instream) ;
        virtual uint8_t getCoupledConf( CONFcouple **couples);

}     ;

#endif
