/***************************************************************************
                          ADM_vidChromaShift.h  -  description
                             -------------------

	Shift chroma to the left or to the right

    begin                : Wed Aug 28 2002
    copyright            : (C) 2002 by mean
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

#ifndef CHROMASHIFT_
#define CHROMASHIFT_

#include "chromashift.h"
/**
    \class ADMVideoChromaShift
*/
 class  ADMVideoChromaShift:public ADM_coreVideoFilter
 {

 protected:
                        ADMImage           *_uncompressed;
                        chromashift         _param;
public:
                        ADMVideoChromaShift(ADM_coreVideoFilter *in,CONFcouple *couples)   ;
                        ~ADMVideoChromaShift();

               
       virtual const char   *getConfiguration(void);          /// Return  current configuration as a human readable string
       virtual bool         getNextFrame(uint32_t *fn,ADMImage *image);    /// Return the next image
	   virtual bool         getCoupledConf(CONFcouple **couples) ;   /// Return the current filter configuration
       virtual bool         configure(void) ;                 /// Start graphical user interface        

      static	bool         shift(uint8_t *target,uint8_t *source, 
                                    uint32_t source_pitch,uint32_t dest_pitch,
                                    uint32_t width, uint32_t height,int32_t val);
      static   bool         shiftPlane(ADM_PLANE plane,ADMImage *s,ADMImage *d,int32_t val);
      static   bool         fixup(ADMImage *target,int32_t val);

 }     ;


#endif
