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
typedef struct FLUX_PARAM
{
	uint32_t temporal_threshold;
	uint32_t spatial_threshold;
	
}FLUX_PARAM;

class  ADMVideoFlux:public AVDMGenericVideoStream
 {

 protected:
    	
        			FLUX_PARAM		*_param;

				void DoFilter_C( uint8_t * currp,  uint8_t * prevp, uint8_t * nextp, 
							 int src_pitch, uint8_t * destp,  int dst_pitch,
							 int row_size,  int height);
				void DoFilter_MMX( uint8_t * currp,  uint8_t * prevp, uint8_t * nextp, 
							 int src_pitch, uint8_t * destp,  int dst_pitch,
							 int row_size,  int height);	 
				int32_t num_frame;
		 		VideoCache		*vidCache;
			
 public:
 		

						ADMVideoFlux(  AVDMGenericVideoStream *in,CONFcouple *setup);

  						 ~ADMVideoFlux();
		        virtual uint8_t 	getFrameNumberNoAlloc(uint32_t frame, uint32_t *len,
          						ADMImage *data,uint32_t *flags);

			virtual uint8_t 	configure( AVDMGenericVideoStream *instream);
     			virtual char 		*printConf(void);
			virtual uint8_t 	getCoupledConf( CONFcouple **couples);
							
 }     ;
#endif
