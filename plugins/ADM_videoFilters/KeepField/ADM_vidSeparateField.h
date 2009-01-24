/*

*/
  class  AVDMVideoSeparateField:public AVDMGenericVideoStream
 {

 protected:

        virtual char 				*printConf(void) ;
						VideoCache	*vidCache;

 public:
 					
  						AVDMVideoSeparateField(  AVDMGenericVideoStream *in,CONFcouple *setup);
  						~AVDMVideoSeparateField();
		      virtual uint8_t 	getFrameNumberNoAlloc(uint32_t frame, uint32_t *len,
          									ADMImage *data,uint32_t *flags);

		      virtual uint8_t 	configure( AVDMGenericVideoStream *instream) {return 1;};

 }     ;
   class  AVDMVideoMergeField:public AVDMGenericVideoStream
 {

 protected:

				VideoCache	*vidCache;
       			 virtual char 		*printConf(void) ;

 public:
  						AVDMVideoMergeField(  AVDMGenericVideoStream *in,CONFcouple *setup);
  						~AVDMVideoMergeField();
		      virtual uint8_t 	getFrameNumberNoAlloc(uint32_t frame, uint32_t *len,
          									ADMImage *data,uint32_t *flags);

			virtual uint8_t configure( AVDMGenericVideoStream *instream) {return 1;};

 }     ;
   class  AVDMVideoStackField:public AVDMGenericVideoStream
 {

 protected:

				
       			 virtual char 		*printConf(void) ;

 public:
  						AVDMVideoStackField(  AVDMGenericVideoStream *in,CONFcouple *setup);
  						~AVDMVideoStackField();
		      virtual uint8_t 	getFrameNumberNoAlloc(uint32_t frame, uint32_t *len,
          									ADMImage *data,uint32_t *flags);

			virtual uint8_t configure( AVDMGenericVideoStream *instream) {return 1;};

 }     ;
 class  AVDMVideoHzStackField:public AVDMGenericVideoStream
 {

 protected:

				
       			 virtual char 		*printConf(void) ;

 public:
  						AVDMVideoHzStackField(  AVDMGenericVideoStream *in,CONFcouple *setup);
  						~AVDMVideoHzStackField();
		      virtual uint8_t 	getFrameNumberNoAlloc(uint32_t frame, uint32_t *len,
          									ADMImage *data,uint32_t *flags);

			virtual uint8_t configure( AVDMGenericVideoStream *instream) {return 1;};

 }     ;
   class  AVDMVideoUnStackField:public AVDMGenericVideoStream
 {

 protected:

                                
                         virtual char           *printConf(void) ;

 public:
                                                AVDMVideoUnStackField(  AVDMGenericVideoStream *in,CONFcouple *setup);
                                                ~AVDMVideoUnStackField();
                      virtual uint8_t   getFrameNumberNoAlloc(uint32_t frame, uint32_t *len,
                                                                                ADMImage *data,uint32_t *flags);

                        virtual uint8_t configure( AVDMGenericVideoStream *instream) {return 1;};

 }     ;

