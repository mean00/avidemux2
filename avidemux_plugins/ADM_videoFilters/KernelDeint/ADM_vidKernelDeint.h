
#ifndef ADM_KERNELDEINT
#define ADM_KERNELDEINT

typedef struct  KERNEL_CONF
{
	uint32_t		order, threshold;
	uint32_t		sharp, twoway, map;

}KERNEL_CONF;

class  ADMVideoKernelDeint:public AVDMGenericVideoStream
 {

 protected:
        virtual char 		*printConf(void) ;
	KERNEL_CONF		*_param;
        int			 debug;
	VideoCache		*vidCache;
	   

 public:
 		
  			ADMVideoKernelDeint(  AVDMGenericVideoStream *in,CONFcouple *setup);
  			~ADMVideoKernelDeint();
	virtual uint8_t getFrameNumberNoAlloc(uint32_t frame, uint32_t *len,
						ADMImage *data,uint32_t *flags);
	virtual uint8_t	getCoupledConf( CONFcouple **couples)		;
	virtual uint8_t configure( AVDMGenericVideoStream *instream);
							
};

#endif
