//
// C++ Interface: ADM_vidSoften
//
// Description: 
//
// Very similar to Avisynth spatial soften
// you might even say inspired by
//
//
// Author: mean <fixounet@free.fr>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef MSOFTEN_
#define MSOFTEN_

typedef struct MaskedSoften_CONF
{
	uint32_t luma,chroma;
	uint32_t radius;
}MaskedSoften_CONF;

 class  ADMVideoMaskedSoften:public AVDMGenericVideoStream
 {

 protected:
    		
		
     virtual 	char 			*printConf(void);
		uint8_t			*_reverse;
		uint8_t 		radius5(uint8_t *_uncompressed, uint8_t *data) ;
		uint8_t 		radius3(uint8_t *_uncompressed, uint8_t *data) ;

 public:
		MaskedSoften_CONF *_param;

  				ADMVideoMaskedSoften(  AVDMGenericVideoStream *in,
							CONFcouple *setup);

  	virtual 		~ADMVideoMaskedSoften();
	virtual uint8_t 	getFrameNumberNoAlloc(uint32_t frame, uint32_t *len,
          						ADMImage *data,uint32_t *flags);
	virtual uint8_t 	configure( AVDMGenericVideoStream *instream) ;
	virtual uint8_t		getCoupledConf( CONFcouple **couples);
	
 }     ;
#endif
