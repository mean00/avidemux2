/***************************************************************************
                          ADM_vidVlad.h  -  description
                             -------------------
    begin                : Fri Jan 3 2003
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
#ifndef __VLAD__
#define __VLAD__   
typedef struct VLAD_PARAM
{
	uint32_t ythresholdMask;
	uint32_t cthresholdMask;
}VLAD_PARAM;
 class  AVDMVideoVlad:public AVDMGenericVideoStream
 {

 protected:

        			VLAD_PARAM	*_param;
        			virtual char 	*printConf(void);
	      			uint8_t		*_mask;
	      			VideoCache	*vidCache;
	      			
        			uint64_t		ythresholdMask;
				uint64_t 		cthresholdMask;
				uint32_t 		num_frame;
				void (*ProcessCPlane)(unsigned char *source,
				                      unsigned char *prev,
				                      unsigned char* dest,
				                      unsigned char* mask,
				                      int width, int height,
				                      uint64_t  threshold);
				void (*ProcessYPlane)(unsigned char *source,
				                      unsigned char *prev,
				                      unsigned char* dest,
				                      unsigned char* mask,
				                      long int width,
				                      long int height,
				                      uint64_t  threshold);
 public:


						AVDMVideoVlad(  AVDMGenericVideoStream *in,CONFcouple *setup);
  					 	~AVDMVideoVlad();
		        virtual uint8_t 	getFrameNumberNoAlloc(uint32_t frame, uint32_t *len,
          							ADMImage *data,uint32_t *flags);

			virtual uint8_t 	configure( AVDMGenericVideoStream *instream);
			virtual uint8_t	getCoupledConf( CONFcouple **couples)		;
 }     ;
#endif
