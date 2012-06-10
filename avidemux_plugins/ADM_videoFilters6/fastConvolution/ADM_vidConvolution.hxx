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

#ifndef ADM_CONVOLUTION_H
#define ADM_CONVOLUTION_H

#include "ADM_default.h"
#include "ADM_coreVideoFilterInternal.h"
#include "DIA_coreToolkit.h"
#include "convolution.h"

/**
    \class AVDMFastVideoConvolution
*/
 class  AVDMFastVideoConvolution:public ADM_coreVideoFilter
 {

 protected:

	virtual uint8_t doLine(uint8_t *pred, uint8_t *cur, uint8_t *next, uint8_t *out, uint32_t w)
                      { UNUSED_ARG(pred); UNUSED_ARG(cur); UNUSED_ARG(next); UNUSED_ARG(out); UNUSED_ARG(w);
                                return 0;}
        
                      convolution	param;
                      ADMImage      *image;
                    bool                processPlane(ADMImage *s,ADMImage *d,ADM_PLANE plane);
 public:
                                        AVDMFastVideoConvolution(ADM_coreVideoFilter *previous,CONFcouple *conf);
                                        ~AVDMFastVideoConvolution();
                    virtual const char   *getConfiguration(void); 
                    virtual bool         getNextFrame(uint32_t *fn,ADMImage *image);    /// Return the next image
                    virtual bool         getCoupledConf(CONFcouple **couples) ;   /// Return the current filter configuration
					virtual void setCoupledConf(CONFcouple *couples);
                    virtual bool         configure(void) ;           /// Start graphical user interface
            

}     ;
/**

*/
 class    AVDMFastVideoMean: public AVDMFastVideoConvolution
 {
        protected:
            virtual uint8_t doLine(uint8_t  *pred,
                                  uint8_t *cur,
                                  uint8_t *next,
                                  uint8_t *out,
                                  uint32_t w)   ;

        public:

                                    AVDMFastVideoMean(  ADM_coreVideoFilter *in,CONFcouple *setup) :
                                    AVDMFastVideoConvolution(  in,setup) {};
              virtual const char 	*getConfiguration(void);

	}      ;

/**

*/
   class    AVDMFastVideoGauss: public AVDMFastVideoConvolution
 {
    protected:
              virtual uint8_t 		doLine(uint8_t  *pred,
                                              uint8_t *cur,
                                              uint8_t *next,
                                              uint8_t *out,
                                              uint32_t w)   ;
  
		public:

                                    AVDMFastVideoGauss(  ADM_coreVideoFilter *in,CONFcouple *setup) :
                                    AVDMFastVideoConvolution(  in,setup) {};
             virtual const char 	*getConfiguration(void);
	}      ;
/**

*/
     class    AVDMFastVideoSharpen: public AVDMFastVideoConvolution
 {
        protected:
            virtual uint8_t 		doLine(uint8_t  *pred,
                                                uint8_t *cur,
                                                uint8_t *next,
                                                uint8_t *out,
                                                uint32_t w)   ;

          public:

                                      AVDMFastVideoSharpen(  ADM_coreVideoFilter *in,CONFcouple *setup) :
                                      AVDMFastVideoConvolution(  in,setup) {};
             virtual const char 	*getConfiguration(void);	
                      
};
/**

*/
  class    AVDMFastVideoMedian: public AVDMFastVideoConvolution
 {
        protected:
          virtual uint8_t 	doLine(uint8_t  *pred,
                                        uint8_t *cur,
                                        uint8_t *next,
                                        uint8_t *out,
                                        uint32_t w)   ;

        public:

                                AVDMFastVideoMedian(  ADM_coreVideoFilter *in,CONFcouple *setup) :
                                AVDMFastVideoConvolution(  in,setup) {};
            virtual const char 	*getConfiguration(void);	
 };

//EOF

#endif
