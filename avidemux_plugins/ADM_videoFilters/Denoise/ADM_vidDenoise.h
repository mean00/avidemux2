/***************************************************************************
                          ADM_vidDenoise.h  -  description
                             -------------------
    begin                : Mon Nov 25 2002
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

 #define DN_COPY 3
 #define DN_LOCK 0
 #define DN_BLEND 1
  
 typedef struct
 {
	  uint32_t lumaThreshold,lumaLock;
	  uint32_t chromaThreshold,chromaLock;
	  uint32_t sceneChange;
	}NOISE_PARAM;
 
  class  ADMVideoDenoise:public AVDMGenericVideoStream
 {

 protected:

      					ADMImage	*_locked;
        				ADMImage	*_lockcount;
   			virtual 	char 		*printConf(void);
        				NOISE_PARAM	*_param;
        				uint32_t	_lastFrame;
        				uint8_t 	doOnePix(uint8_t *in,uint8_t *out,
        							uint8_t *lock,uint8_t *nb);
         				uint8_t 	doBlend(uint8_t *in,uint8_t *out,
        							uint8_t *lock,uint8_t *nb);

 public:


							ADMVideoDenoise(  AVDMGenericVideoStream *in,CONFcouple *setup);
  			virtual 			~ADMVideoDenoise();
		        virtual uint8_t 		getFrameNumberNoAlloc(uint32_t frame, uint32_t *len,
          								ADMImage *data,uint32_t *flags);
			virtual uint8_t 		configure( AVDMGenericVideoStream *instream) ;

			virtual uint8_t			getCoupledConf( CONFcouple **couples);
 }     ;

