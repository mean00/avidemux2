/***************************************************************************
                          ADM_vidAddBorder.h  -  description
                             -------------------
    begin                : Sun Aug 11 2002
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
#ifndef __ADDBORDER__
#define     __ADDBORDER__


typedef struct
{
        uint32_t left,right;
        uint32_t top,bottom;
}ADDBORDER_PARAMS;

  class  AVDMVideoAddBorder:public AVDMGenericVideoStream
 {

 protected:
              virtual char        *printConf(void);
              ADDBORDER_PARAMS         *_param;
 public:

                                    AVDMVideoAddBorder(  AVDMGenericVideoStream *in,CONFcouple *setup);
                                    AVDMVideoAddBorder(  AVDMGenericVideoStream *in,uint32_t x,uint32_t x2,
                                                              uint32_t y,uint32_t y2);
            virtual                 ~AVDMVideoAddBorder();
            virtual 	uint8_t     getFrameNumberNoAlloc(uint32_t frame, uint32_t *len,
                                            ADMImage *data,uint32_t *flags);
                        uint8_t     configure( AVDMGenericVideoStream *instream) ;
                        uint8_t     getCoupledConf( CONFcouple **couples);
 }     ;

#endif
