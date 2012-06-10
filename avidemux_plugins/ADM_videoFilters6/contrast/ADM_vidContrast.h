/***************************************************************************
                          ADM_vidContrast.h  -  description
                             -------------------
    begin                : Sun Sep 22 2002
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
#ifndef _CONTRAST_
#define _CONTRAST_
#include "contrast.h"
bool    doContrast (ADMImage * in, ADMImage * out, uint8_t * table,  ADM_PLANE plane);	
uint8_t buildContrastTable( float coef,int8_t off, uint8_t *tableFlat,uint8_t *tableNZ);


/**
	\class ADMVideoContrast
	\brief Contrast video filter
*/
 class  ADMVideoContrast:public ADM_coreVideoFilter
 {

 protected:
				contrast					    _param;
				uint8_t						_tableFlat[256];
				uint8_t						_tableNZ[256];
 public:
                            ADMVideoContrast(ADM_coreVideoFilter *in,CONFcouple *couples)   ;
      virtual              ~ADMVideoContrast();

               void          update(void); 
       virtual const char   *getConfiguration(void);          /// Return  current configuration as a human readable string
       virtual bool         getNextFrame(uint32_t *fn,ADMImage *image);    /// Return the next image
	   virtual bool         getCoupledConf(CONFcouple **couples) ;   /// Return the current filter configuration
	   virtual void setCoupledConf(CONFcouple *couples);
       virtual bool         configure(void) ;                 /// Start graphical user interface        

 				
 }     ;
#endif
