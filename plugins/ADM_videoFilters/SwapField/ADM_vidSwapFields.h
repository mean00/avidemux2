/***************************************************************************
                          Swap Fields.h  -  description
                             -------------------
Swap each line  (shift up for odd, down for even)


    begin                : Thu Mar 21 2002
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


  class  AVDMVideoSwapField:public AVDMGenericVideoStream
 {

 protected:
    		AVDMGenericVideoStream 	*_in;
        virtual char 					*printConf(void) ;

 public:
  						AVDMVideoSwapField(  AVDMGenericVideoStream *in,CONFcouple *setup);
  						~AVDMVideoSwapField();
		      virtual uint8_t 	getFrameNumberNoAlloc(uint32_t frame, uint32_t *len,
          									ADMImage *data,uint32_t *flags);
			virtual uint8_t	*getBinaryConf( uint16_t *size) { *size=0;return (uint8_t *)0;}
			virtual uint8_t 	configure( AVDMGenericVideoStream *instream) {return 1;};

 }     ;
  class  AVDMVideoKeepOdd:public AVDMGenericVideoStream
 {

 protected:
    		AVDMGenericVideoStream 	*_in;
        virtual char 					*printConf(void) ;

 public:
  						AVDMVideoKeepOdd(  AVDMGenericVideoStream *in,CONFcouple *setup);
  						~AVDMVideoKeepOdd();
		      virtual uint8_t 	getFrameNumberNoAlloc(uint32_t frame, uint32_t *len,
          									ADMImage *data,uint32_t *flags);

			virtual uint8_t 	configure( AVDMGenericVideoStream *instream) {return 1;};

 }     ;
  class  AVDMVideoKeepEven:public AVDMVideoKeepOdd
 {

 protected:

       			 virtual char 					*printConf(void) ;

 public:
 			AVDMVideoKeepEven(  AVDMGenericVideoStream *in,CONFcouple *setup) : AVDMVideoKeepOdd(  in,setup) 
			{}

		      virtual uint8_t 	getFrameNumberNoAlloc(uint32_t frame, uint32_t *len,
          							ADMImage *data,uint32_t *flags);

 }     ;

