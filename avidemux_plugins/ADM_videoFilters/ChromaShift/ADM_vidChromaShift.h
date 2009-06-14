/***************************************************************************
                          ADM_vidChromaShift.h  -  description
                             -------------------

	Shift chroma to the left or to the right

    begin                : Wed Aug 28 2002
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

#ifndef CHROMASHIFT_
#define CHROMASHIFT_

#include "ADM_vidChromaShift_param.h"
 class  ADMVideoChromaShift:public AVDMGenericVideoStream
 {

 protected:


            virtual 	char               *printConf(void);
                        CHROMASHIFT_PARAM *_param;

 public:
                        ADMVideoChromaShift(  AVDMGenericVideoStream *in,CONFcouple *setup);
      virtual           ~ADMVideoChromaShift();
      virtual uint8_t   getFrameNumberNoAlloc(uint32_t frame, uint32_t *len,
                                                              ADMImage *data,uint32_t *flags);
      virtual uint8_t   configure( AVDMGenericVideoStream *instream);
      virtual uint8_t   getCoupledConf( CONFcouple **couples);

      static	uint8_t shift(uint8_t *target,uint8_t *source,
                                                      uint32_t width, uint32_t height,
                                                      int32_t val);
      static uint8_t    fixup(uint8_t *target,uint32_t width, uint32_t height,int32_t val);

 }     ;


#endif
