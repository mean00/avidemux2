/***************************************************************************
                          ADM_mpeg2enc.h  -  description
                             -------------------

		Interface to mpegenc

    begin                : Tue Jun 4 2002
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
#ifndef ADM_MPEGENC
#define ADM_MPEGENC
#include "mpeg2parm.h"
#include "ADM_mpeg2Param.h"


#define MPG_4_3 	2
#define MPG_16_9 	3
class Mpeg2enc: public encoder
  	{
     	protected :

				mpeg2parm	 	_settings;
				uint8_t			_firstFrame;
			    uint8_t         setQz( uint32_t quant);
			    uint8_t         commonMpeg2Init( uint32_t qz, uint32_t maxbr ,uint32_t fps1000
				                        ,uint8_t interlaced, uint8_t bff, uint8_t wide,	uint8_t fast);
          public :
          			   Mpeg2enc(uint32_t width,uint32_t height);
		  virtual         ~Mpeg2enc();
                  virtual       uint8_t stopEncoder(void );
		  virtual	uint8_t init(uint32_t a, uint32_t b) {return 1;};
                  virtual 	uint8_t init(  uint32_t qz, uint32_t maxbr, uint32_t fps1000,
						uint8_t interlaced, uint8_t bff, uint8_t wide,
						uint8_t fast); // WLA

                  virtual 	uint8_t encode(ADMImage *in,ADMBitstream *out);
		virtual 	uint8_t setMatrix(int matrix);	
				uint8_t	disablePadding( void );
				uint32_t getCodedPictureNumber( void );
};


class Mpeg2encVCD: public Mpeg2enc
  	{
     	protected :


          public :
          			Mpeg2encVCD(uint32_t width,uint32_t height);
    				virtual 	uint8_t init(  uint32_t qz, uint32_t maxbr, uint32_t fps1000,
									uint8_t interlaced, uint8_t bff, uint8_t wide,
									uint8_t fast); // WLA

     };
class Mpeg2encSVCD: public Mpeg2enc
  	{
     	protected :


          public :
          			Mpeg2encSVCD(uint32_t width,uint32_t height);
  				virtual 	uint8_t init( uint32_t qz, uint32_t maxbr, uint32_t fps1000,
									uint8_t interlaced, uint8_t bff, uint8_t wide,
									uint8_t fast); // WLA
     };
class Mpeg2encDVD: public Mpeg2enc
  	{
     	protected :


          public :
          			Mpeg2encDVD(uint32_t width,uint32_t height);
  				virtual 	uint8_t init( uint32_t qz, uint32_t maxbr, uint32_t fps1000,
									uint8_t interlaced, uint8_t bff, uint8_t wide,
									uint8_t fast); // WLA
     };

int mpegenc_init(mpeg2parm *incoming,int width, int height, int fps1000);
int mpegenc_encode(  char *in,   char *out, int *size,int *flags,int *quant);
int mpegenc_setQuantizer(int q);

int mpegenc_end(void);

#endif

