#ifndef ADM_VIDEO_NULL_H
#define ADM_VIDEO_NULL_H

class  AVDMVideoStreamNull :public AVDMGenericVideoStream
 {

 protected:
                  ADM_Composer      *_in;
                  uint32_t          _start;
                  uint32_t          par_width,par_height;

 public:

                          AVDMVideoStreamNull(  ADM_Composer *in,uint32_t framestart, uint32_t nb);
                          virtual ~AVDMVideoStreamNull();
    
    
        virtual	  uint8_t getFrameNumberNoAlloc(uint32_t frame, uint32_t *len,
                                                          ADMImage *data,uint32_t *flags);
                  uint8_t configure( AVDMGenericVideoStream *instream);
    
        virtual uint32_t   getPARWidth(void);
        virtual uint32_t   getPARHeight(void);

 }     ;

class  AVDMVideoStreamRaw :public AVDMGenericVideoStream
 {

 protected:
				ADM_Composer *_in;
				uint32_t			_start;

 public:

  				AVDMVideoStreamRaw(  ADM_Composer *in,uint32_t framestart, uint32_t nb);
  				virtual ~AVDMVideoStreamRaw();


          virtual 	uint8_t getFrameNumberNoAlloc(uint32_t frame, uint32_t *len,
								ADMImage *data,uint32_t *flags);
           	 	uint8_t configure( AVDMGenericVideoStream *instream);


 }     ;

#endif
// EOF
