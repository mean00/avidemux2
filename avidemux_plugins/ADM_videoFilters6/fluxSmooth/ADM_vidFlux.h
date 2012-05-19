/***************************************************************************
                          ADM_vidFlux.h  -  description
                             -------------------
    begin                : Tue Dec 31 2002
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
 
#ifndef __FLUX__
#define __FLUX__   
#include "fluxsmooth.h"
/**
    \class ADMVideoFlux
*/

typedef void DoFlux( uint8_t * currp,  uint8_t * prevp, uint8_t * nextp, 
							 int src_pitch, uint8_t * destp,  int dst_pitch,
							 int row_size,  int height, const fluxsmooth &_param);


class  ADMVideoFlux:public ADM_coreVideoFilterCached
 {

 protected:
    	
        			fluxsmooth		_param;

				static void DoFilter_C( uint8_t * currp,  uint8_t * prevp, uint8_t * nextp, 
							 int src_pitch, uint8_t * destp,  int dst_pitch,
							 int row_size,  int height, const fluxsmooth &_param);
				static void DoFilter_MMX( uint8_t * currp,  uint8_t * prevp, uint8_t * nextp, 
							 int src_pitch, uint8_t * destp,  int dst_pitch,
							 int row_size,  int height, const fluxsmooth &_param);	 
				int32_t num_frame;
		 		
			
 public:
 		
                            ADMVideoFlux(ADM_coreVideoFilter *in,CONFcouple *couples);    
                             ~ADMVideoFlux(void);
       virtual const char  *getConfiguration(void);          /// Return  current configuration as a human readable string
       virtual bool         getNextFrame(uint32_t *fn,ADMImage *image);    /// Return the next image
       virtual bool         getCoupledConf(CONFcouple **couples) ;   /// Return the current filter configuration
       virtual bool         configure(void) ;                 /// Start graphical user interface        
   
							
 }     ;
#endif
