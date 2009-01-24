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
#ifndef ADM_VID_COLOR_YUV_H
#define ADM_VID_COLOR_YUV_H
#include "ADM_vidColorYuv_param.h"
class  ADMVideoColorYuv:public AVDMGenericVideoStream
{

    protected:

        virtual char    *printConf(void) ;
        COLOR_YUV_PARAM *_param;
        
        void            MakeGammaLUT(void);
        
        uint8_t      LUT_Y[256],LUT_U[256],LUT_V[256];
        uint32_t     accum_Y[256],accum_U[256],accum_V[256];
        int32_t      y_thresh1, y_thresh2, u_thresh1, u_thresh2, v_thresh1, v_thresh2;
        int32_t      last_y_offset;

        
    public:
 					
        ADMVideoColorYuv(  AVDMGenericVideoStream *in,CONFcouple *setup);
        ~ADMVideoColorYuv();
        virtual uint8_t 	getFrameNumberNoAlloc(uint32_t frame, uint32_t *len,
                ADMImage *data,uint32_t *flags);

        virtual uint8_t configure( AVDMGenericVideoStream *instream) ;
        virtual uint8_t getCoupledConf( CONFcouple **couples);

}     ;

#endif
