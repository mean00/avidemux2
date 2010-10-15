/***************************************************************************
                          ADM_vidLargeMedian.h  -  description
                             -------------------
    begin                : Wed Jan 1 2003
    copyright            : (C) 2003 by mean
    email                : fixounet@free.fr
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef  LARGE_MEDIAN_H
#define  LARGE_MEDIAN_H
#include "convolution.h"

/**
    \class largeMedian
*/
class largeMedian : public  ADM_coreVideoFilter
{
protected:
        convolution  param;
virtual uint8_t 	 doLine(uint8_t  *pred2,uint8_t *pred1,	uint8_t *cur,uint8_t *next1,uint8_t *next2,
   										uint8_t *out,uint32_t w)    ;
                bool        processPlane(ADMImage *s,ADMImage *d,ADM_PLANE plane);
                ADMImage    *image;
public:
                    largeMedian(ADM_coreVideoFilter *previous,CONFcouple *conf);
                    ~largeMedian();

        virtual const char   *getConfiguration(void);                   /// Return  current configuration as a human readable string
        virtual bool         getNextFrame(uint32_t *fn,ADMImage *image);    /// Return the next image
	 //  virtual FilterInfo  *getInfo(void);                             /// Return picture parameters after this filter
        virtual bool         getCoupledConf(CONFcouple **couples) ;   /// Return the current filter configuration
        virtual bool         configure(void) ;           /// Start graphical user interface
};

#endif
// EOF

