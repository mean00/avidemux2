/***************************************************************************
                          ADM_vidMPLD3D.h  -  description
                             -------------------
Mplayer Denoise3d port to avidemux2
Original Authors
Copyright (C) 2003
Daniel Moreno <comac@comac.darktech.org>


 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef __D3DLOW__
#define __D3DLOW__

#include "ADM_vidMPLD3D.h"

class  ADMVideoMPD3Dlow:public AVDMGenericVideoStream
 {

 protected:

        			MPD3D_PARAM			*_param;

				int				Coefs[4][512];
        			uint8_t				*Line;
				ADMImage			*_stored;
				


				uint32_t					_last;

				void 	PrecalcCoefs(int *Ct, double Dist25);
				uint8_t  	setup(void);
				void 	deNoise(unsigned char *Frame,        // mpi->planes[x]
                    						unsigned char *FramePrev,    // pmpi->planes[x]
                    						unsigned char *FrameDest,    // dmpi->planes[x]
                    						unsigned char *LineAnt,      // vf->priv->Line (width bytes)
                    						int W, int H, int sStride, int pStride, int dStride,
                    						int *Horizontal, int *Vertical, int *Temporal);



 public:


						ADMVideoMPD3Dlow(  AVDMGenericVideoStream *in,CONFcouple *setup);
  						 ~ADMVideoMPD3Dlow();
		        virtual uint8_t 	getFrameNumberNoAlloc(uint32_t frame, uint32_t *len,
 									ADMImage *data,uint32_t *flags);

			virtual uint8_t 	configure( AVDMGenericVideoStream *instream);
     			virtual char 		*printConf(void);
			virtual uint8_t 	getCoupledConf( CONFcouple **couples);

 }     ;
#endif
