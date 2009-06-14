/***************************************************************************
                          ADM_vidMPLD3D.h  -  description
                             -------------------
Mplayer HQDenoise3d port to avidemux2
Original Authors
Copyright (C) 2003
Daniel Moreno <comac@comac.darktech.org>
	& A'rpi

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
 
#ifndef _D3D__
#define _D3D__   

typedef struct MPD3D_PARAM
{
	double  param1;  // Luma 
	double  param2;  // Chroma
	double  param3;  // Temporal
}MPD3D_PARAM;

class  ADMVideoMPD3D:public AVDMGenericVideoStream
 {

 protected:

        			MPD3D_PARAM			*_param;

				int	  					Coefs[4][512*16];
        			uint32_t					*Line;

				uint16_t				*_uncompressed;
				ADMImage				*_storage;
				uint32_t				_last;

				void 	PrecalcCoefs(int *Ct, double Dist25);
				uint8_t  	setup(void);
				void 	deNoise(unsigned char *Frame,        // mpi->planes[x]
                    					unsigned char *FrameDest,    // dmpi->planes[x]
                    					uint32_t *LineAnt,      // vf->priv->Line (width bytes)
		    					unsigned short *FrameAntPtr,
                    					int W, int H, int sStride, int dStride,
                    					int *Horizontal, int *Vertical, int *Temporal);



 public:


						ADMVideoMPD3D(  AVDMGenericVideoStream *in,CONFcouple *setup);
  						 ~ADMVideoMPD3D();
		        virtual uint8_t 	getFrameNumberNoAlloc(uint32_t frame, uint32_t *len,
   								ADMImage *data,uint32_t *flags);

			virtual uint8_t 	configure( AVDMGenericVideoStream *instream);
     			virtual char 	*printConf(void);
			virtual uint8_t 	getCoupledConf( CONFcouple **couples);
							
 }     ;
#endif
