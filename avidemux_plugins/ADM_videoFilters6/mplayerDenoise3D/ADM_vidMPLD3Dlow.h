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

#include "denoise3d.h"

/**
    \class ADMVideoMPD3Dlow
*/
class ADMVideoMPD3Dlow : public  ADM_coreVideoFilterCached
{
protected:
                ADMImage        *original;
                denoise3d       param;
                uint32_t        last;
                uint8_t         *Line;
                int				Coefs[4][512];
                void 	    PrecalcCoefs(int *Ct, double Dist25);
				uint8_t  	setup(void);
				void 	    deNoise(unsigned char *Frame,        // mpi->planes[x]
                    						unsigned char *FramePrev,    // pmpi->planes[x]
                    						unsigned char *FrameDest,    // dmpi->planes[x]
                    						unsigned char *LineAnt,      // vf->priv->Line (width bytes)
                    						int W, int H, int sStride, int pStride, int dStride,
                    						int *Horizontal, int *Vertical, int *Temporal);

                    
public:
                    ADMVideoMPD3Dlow(ADM_coreVideoFilter *previous,CONFcouple *conf);
                    ~ADMVideoMPD3Dlow();

        virtual const char   *getConfiguration(void);                 /// Return  current configuration as a human readable string
        virtual bool         getNextFrame(uint32_t *fn,ADMImage *image);           /// Return the next image
        virtual bool         getCoupledConf(CONFcouple **couples) ;   /// Return the current filter configuration
        virtual bool         configure(void) ;                        /// Start graphical user interface
};

			

#endif
