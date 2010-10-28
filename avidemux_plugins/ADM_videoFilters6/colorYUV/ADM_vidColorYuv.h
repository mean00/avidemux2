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

#include "colorYuv.h"
/**
    \class vidColorYuv
*/
class vidColorYuv : public  ADM_coreVideoFilter
{
protected:
        uint8_t       LUT_Y[256],LUT_U[256],LUT_V[256];
        int           y_thresh1, y_thresh2, u_thresh1, u_thresh2, v_thresh1, v_thresh2;

protected:
        colorYuv    param;
        void        MakeGammaLUT(void);
public:
        vidColorYuv(ADM_coreVideoFilter *previous,CONFcouple *conf);
        ~vidColorYuv();

        virtual const char   *getConfiguration(void);                   /// Return  current configuration as a human readable string
        virtual bool         getNextFrame(uint32_t *fn,ADMImage *image);    /// Return the next image
	 //  virtual FilterInfo  *getInfo(void);                             /// Return picture parameters after this filter
        virtual bool         getCoupledConf(CONFcouple **couples) ;   /// Return the current filter configuration
        virtual bool         configure(void) ;           /// Start graphical user interface
};



#endif
