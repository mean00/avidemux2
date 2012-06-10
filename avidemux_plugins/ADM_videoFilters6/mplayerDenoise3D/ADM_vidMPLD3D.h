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

#include "denoise3dHQ.h"
typedef struct {
    int Coefs[4][512*16];
    unsigned int *Line;
    unsigned short *Frame[3];
    int hsub, vsub;
} HQDN3DContext;

#define PARAM1_DEFAULT 4.0
#define PARAM2_DEFAULT 3.0
#define PARAM3_DEFAULT 6.0

/**
    \class ADMVideoMPD3Dlow
*/
class ADMVideoMPD3D : public  ADM_coreVideoFilterCached
{
protected:
                ADMImage                        *original;
                denoise3dhq                     param;
                HQDN3DContext                   context;
                uint32_t                        last;
                uint8_t  	                setup(void);
			
                    
public:
                    ADMVideoMPD3D(ADM_coreVideoFilter *previous,CONFcouple *conf);
                    ~ADMVideoMPD3D();

        virtual const char   *getConfiguration(void);                 /// Return  current configuration as a human readable string
        virtual bool         getNextFrame(uint32_t *fn,ADMImage *image);           /// Return the next image
        virtual bool         getCoupledConf(CONFcouple **couples) ;   /// Return the current filter configuration
		virtual void setCoupledConf(CONFcouple *couples);
        virtual bool         configure(void) ;                        /// Start graphical user interface
        virtual bool         goToTime(uint64_t usSeek);  
};

			

#endif
