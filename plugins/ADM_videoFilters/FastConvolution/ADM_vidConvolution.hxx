/***************************************************************************
                          ADM_vidConvolution.hxx  -  description
                             -------------------
    begin                : Wed Apr 10 2002
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
 #ifndef __CONVPARAM__
 #define __CONVPARAM__
 typedef struct CONV_PARAM
 {
        uint32_t luma;
        uint32_t chroma;
  }CONV_PARAM;
 #endif

//---------------------------------------------------------
 class  AVDMFastVideoConvolution:public AVDMGenericVideoStream
 {

 protected:

	virtual uint8_t doLine(uint8_t *pred, uint8_t *cur, uint8_t *next, uint8_t *out, uint32_t w)
                      { UNUSED_ARG(pred); UNUSED_ARG(cur); UNUSED_ARG(next); UNUSED_ARG(out); UNUSED_ARG(w); return 0;}
        
                CONV_PARAM	*_param;
 public:
#warning FIXME: string constants cannot be used as char
                      virtual char     *printConf(void) { return const_cast<char*>("ERROR");};

                                         AVDMFastVideoConvolution(  AVDMGenericVideoStream *in,CONFcouple *setup);
                      virtual           ~AVDMFastVideoConvolution();
                      virtual uint8_t   getFrameNumberNoAlloc(uint32_t frame, uint32_t *len,
                                                      ADMImage *data,uint32_t *flags);
                      virtual uint8_t	configure( AVDMGenericVideoStream *instream) ;

                      virtual uint8_t	getCoupledConf( CONFcouple **couples);
}     ;

 class    AVDMFastVideoMean: public AVDMFastVideoConvolution
 {
        protected:
            virtual uint8_t doLine(uint8_t  *pred,
                                  uint8_t *cur,
                                  uint8_t *next,
                                  uint8_t *out,
                                  uint32_t w)   ;

        public:

                              AVDMFastVideoMean(  AVDMGenericVideoStream *in,CONFcouple *setup) :
                              AVDMFastVideoConvolution(  in,setup) {};
              virtual char 	*printConf(void);

	}      ;


   class    AVDMFastVideoGauss: public AVDMFastVideoConvolution
 {
    protected:
              virtual uint8_t 		doLine(uint8_t  *pred,
                                              uint8_t *cur,
                                              uint8_t *next,
                                              uint8_t *out,
                                              uint32_t w)   ;
  
		public:

                                    AVDMFastVideoGauss(  AVDMGenericVideoStream *in,CONFcouple *setup) :
                                    AVDMFastVideoConvolution(  in,setup) {};
                    virtual char	*printConf(void);

	}      ;
     class    AVDMFastVideoSharpen: public AVDMFastVideoConvolution
 {
        protected:
            virtual uint8_t 		doLine(uint8_t  *pred,
                                                uint8_t *cur,
                                                uint8_t *next,
                                                uint8_t *out,
                                                uint32_t w)   ;

          public:

                                      AVDMFastVideoSharpen(  AVDMGenericVideoStream *in,CONFcouple *setup) :
                                      AVDMFastVideoConvolution(  in,setup) {};
                              virtual char 		*printConf(void);	
                      
  }      ;

  class    AVDMFastVideoMedian: public AVDMFastVideoConvolution
 {
        protected:
          virtual uint8_t 	doLine(uint8_t  *pred,
                                        uint8_t *cur,
                                        uint8_t *next,
                                        uint8_t *out,
                                        uint32_t w)   ;

        public:

                                AVDMFastVideoMedian(  AVDMGenericVideoStream *in,CONFcouple *setup) :
                                AVDMFastVideoConvolution(  in,setup) {};
#warning FIXME: string constants cannot be used as char
								virtual char     *printConf(void) { return const_cast<char*>("ERROR");};
 }      ;

//EOF

