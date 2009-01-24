//
// C++ Interface: %{MODULE}
//
// Description: 
//
//
// Author: %{AUTHOR} <%{EMAIL}>, (C) %{YEAR}
//
// Copyright: See COPYING file that comes with this distribution
//
//
typedef struct EqualizerParam
{
	uint32_t _scaler[256];
}EqualizerParam;

 class  vidEqualizer:public AVDMGenericVideoStream
 {

 protected:


           virtual char 	*printConf(void);
			EqualizerParam *_param;	
			

 public:
 		
  					vidEqualizer(  AVDMGenericVideoStream *in,CONFcouple *setup);	
  			virtual 	~vidEqualizer();
			virtual uint8_t getFrameNumberNoAlloc(uint32_t frame, uint32_t *len,
			  		ADMImage *data,uint32_t *flags);
				uint8_t configure( AVDMGenericVideoStream *instream);
			virtual uint8_t	getCoupledConf( CONFcouple **couples);
							
 }     ;
 
 uint8_t equalizerBuildScaler(int32_t *p,uint32_t *s);
 
//EOF
