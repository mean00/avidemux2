//
// C++ Interface: ADM_vidForcedPP
//
// Description: 
//
//
// Author: mean <fixounet@free.fr>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef ADM_FORCEDPP
#define ADM_FORCEDPP
#include "ADM_pp.h"

typedef struct  PP_CONF
{
	uint32_t			postProcType;
	uint32_t			postProcStrength;
	uint32_t			forcedQuant;
}PP_CONF;

class  ADMVideoForcedPP:public AVDMGenericVideoStream
 {

 protected:
        virtual char 		*printConf(void) ;
	PP_CONF			*_param;
	ADM_PP			_postproc;
           

 public:
 		
  			ADMVideoForcedPP(  AVDMGenericVideoStream *in,CONFcouple *setup);
  			~ADMVideoForcedPP();
	virtual uint8_t getFrameNumberNoAlloc(uint32_t frame, uint32_t *len,
						ADMImage *data,uint32_t *flags);
	virtual uint8_t	getCoupledConf( CONFcouple **couples)		;
	virtual uint8_t configure( AVDMGenericVideoStream *instream);
							
};

#endif
