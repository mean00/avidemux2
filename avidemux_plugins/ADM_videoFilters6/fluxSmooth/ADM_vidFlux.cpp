/***************************************************************************
                          ADM_vidFlux.cpp  -  description
                             -------------------
    begin                : Tue Dec 31 2002
    copyright            : (C) 2002 by mean
    email                : fixounet@free.fr
    
    Ported from FluxSmooth
    (c)  Ross Thomas <ross@grinfinity.com>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
 
#include <math.h>


#include "ADM_default.h"
#include "DIA_factory.h"
#include "ADM_coreVideoFilter.h"
#include "ADM_vidFlux.h"
#include "fluxsmooth_desc.cpp"
extern void initScaleTab( void );
//********** Register chunk ************

DECLARE_VIDEO_FILTER(   ADMVideoFlux,   // Class
                        1,0,0,              // Version
                        ADM_UI_ALL,         // UI
                        VF_NOISE,            // Category
                        "fluxsmooth",            // internal name (must be uniq!)
                        QT_TRANSLATE_NOOP("flux","FluxSmooth"),            // Display name
                        QT_TRANSLATE_NOOP("flux","Spatio-temporal cleaner by Ross Thomas.") // Description
                    );
//********** /Register chunk ************

/**
    \fn ctor
*/

ADMVideoFlux::ADMVideoFlux(ADM_coreVideoFilter *in,CONFcouple *couples) :
ADM_coreVideoFilterCached(5,in,couples)
			
{
    initScaleTab();
	 if(!couples || !ADM_paramLoad(couples,fluxsmooth_param,&_param))
	{
		 
		 _param.spatial_threshold=7;
		 _param.temporal_threshold=7;
	}
  	num_frame=0xffff0000;
}
/**
    \fn dtor
*/
ADMVideoFlux::~ADMVideoFlux(void)
{
                
}
/**
    \fn getCoupledConf
*/
bool ADMVideoFlux::getCoupledConf( CONFcouple **couples)
{
    return ADM_paramSave(couples, fluxsmooth_param,&_param);
}

void ADMVideoFlux::setCoupledConf(CONFcouple *couples)
{
    ADM_paramLoad(couples, fluxsmooth_param, &_param);
}


/**
    \fn configure
*/
bool ADMVideoFlux::configure()
{
uint8_t r;
#define PX(X) &(_param.X##_threshold)

    diaElemUInteger Gtemporal(PX(temporal),QT_TRANSLATE_NOOP("flux","_Temporal threshold:"),0,255);
    diaElemUInteger Gspatial(PX(spatial),QT_TRANSLATE_NOOP("flux","_Spatial threshold:"),0,255);
	  
    diaElem *elems[2]={&Gtemporal,&Gspatial};
  
    r=diaFactoryRun(QT_TRANSLATE_NOOP("flux","FluxSmooth"),2,elems);
    return r;
    
}

/**
    \fn getConfiguration
    \brief Return current setting as a string
*/
const char *ADMVideoFlux::getConfiguration(void)
{
    static char conf[80];
    conf[0]=0;
    snprintf(conf,80,"FluxSmooth: Spatial :%02"PRIu32" Temporal:%02"PRIu32,
						_param.spatial_threshold,
						_param.temporal_threshold);
    return conf;
}

/**
    \fn getNextFrame
*/
bool         ADMVideoFlux::getNextFrame(uint32_t *fn,ADMImage *output)
{
ADMImage	*image,*next,*prev;
int frame=nextFrame++;
			
			image=vidCache->getImage(frame);
            *fn=frame;
			if(!image) return false;
			

			next=vidCache->getImage(frame+1);
			if(!frame  || !next) // first or last image
			{
				output->duplicate(image);
				output->copyInfo(image);
				vidCache->unlockAll();
				return true;
			}

			prev=vidCache->getImage(frame-1);
            ADM_assert(prev);

		   	DoFlux *flux=	DoFilter_C;	
#if defined(ADM_CPU_X86) && defined(ASM_FLUX)
            flux=DoFilter_MMX;
#endif
// now we have everything
        for(int i=0;i<3;i++)
        {
            ADM_PLANE plane=(ADM_PLANE)i;
            int dst_pitch = output->GetPitch(plane),
                src_pitch = image->GetPitch(plane),
                row_size  = image->GetWidth(plane),
                height    = image->GetHeight(plane);

            uint8_t   		*currp = image->GetReadPtr(plane),
                            *prevp = prev->GetReadPtr(plane),
                            *nextp = next->GetReadPtr(plane);
            uint8_t		*destp =output->GetWritePtr(plane);

                    // line 1 and last
                    memcpy(destp, currp, row_size);
                    memcpy(destp + dst_pitch * (height - 1),
                        currp + src_pitch * (height - 1), row_size);

                    // skip one line		
                    currp += src_pitch;
                    prevp += src_pitch;
                    nextp += src_pitch;
                    destp += dst_pitch;
                    
                    flux(currp, prevp, nextp, src_pitch,
                            destp, dst_pitch, row_size, height - 2,_param);


         }
        output->copyInfo(image);
        vidCache->unlockAll();
        return 1;
}	                           

//
