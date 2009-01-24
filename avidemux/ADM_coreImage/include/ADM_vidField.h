/***************************************************************************
                          ADM_vidField.h  -  description
                             -------------------
    begin                : Sun Jan 12 2003
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
#ifndef _VIDFIELD_
#define _VIDFIELD_
 
#define ASM_DEINT
#define ASM_BLEND

#define THRES1 15*15
#define THRES2 9*9

typedef struct DEINT_PARAM
{
	  uint32_t motion_trigger;
	  uint32_t blend_trigger;

} DEINT_PARAM;

 class  ADMVideoFields:public AVDMGenericVideoStream
 {

 protected:
 		DEINT_PARAM					*_param;

      		uint8_t						*_motionmask;
        	uint8_t						*_motionmask2;
           	virtual char 					*printConf(void) { assert(0);return NULL;}
           	uint8_t 						hasMotion(ADMImage *image);
		void  							hasMotion_C(uint8_t *p,uint8_t *c,
											uint8_t *n,
											uint8_t *e,
											uint8_t *e2
											);
		void  							  hasMotion_MMX(uint8_t *p,uint8_t *c,
											uint8_t *n,
											uint8_t *e,
											uint8_t *e2
												);

           uint8_t 							doBlend(ADMImage *src,
	   									ADMImage *dst);
           void							blend_C(uint8_t *p,uint8_t *c,
											uint8_t *n,
											uint8_t *e,
											uint8_t *f
											);
	  void								blend_MMX(uint8_t *p,uint8_t *c,
													uint8_t *n,
													uint8_t *e,
													uint8_t *f
											);

 public:


							ADMVideoFields(  AVDMGenericVideoStream *in,CONFcouple *setup);
  				virtual 		~ADMVideoFields();
		     		virtual uint8_t 	getFrameNumberNoAlloc(uint32_t frame, uint32_t *len,
          									ADMImage *data,uint32_t *flags) 
								{
								 UNUSED_ARG(frame); UNUSED_ARG(len); UNUSED_ARG(data);
								 UNUSED_ARG(flags); assert(0);return 0;
								 }
					virtual uint8_t configure( AVDMGenericVideoStream *instream) ;

					virtual uint8_t 	getCoupledConf( CONFcouple **couples);
 }     ;

#endif
