/***************************************************************************
                          Hue/Saturation filter ported from mplayer 
 (c) Michael Niedermayer
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#define _USE_MATH_DEFINES // some compilers do not export M_PI etc.. if GNU_SOURCE or that is defined, let's do that
#include <math.h>
#include "ADM_default.h"
#include "ADM_coreVideoFilter.h"
#include "ADM_coreVideoFilterInternal.h"
#include "ADM_vidHue.h"
#include "DIA_factory.h"
#include "hue.h"
#include "hue_desc.cpp"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

extern uint8_t DIA_getHue(hue *param, ADM_coreVideoFilter *in);

// Add the hook to make it valid plugin
DECLARE_VIDEO_FILTER(   ADMVideoHue,   // Class
                        1,0,0,              // Version
                        ADM_UI_TYPE_BUILD,         // UI
                        VF_COLORS,            // Category
                        "hue",            // internal name (must be uniq!)
                        QT_TRANSLATE_NOOP("hue","Mplayer Hue"),            // Display name
                        QT_TRANSLATE_NOOP("hue","Adjust hue and saturation.") // Description
                    );
/**
    \fn HueProcess_C
*/
void ADMVideoHue::HueProcess_C(uint8_t *udst, uint8_t *vdst, uint8_t *usrc, uint8_t *vsrc, int dststride, int srcstride,
                                int w, int h, const int s, const int c)
{
	int i;
#if 0
	const int s= (int)rint(sin(hue) * (1<<16) * sat);
	const int c= (int)rint(cos(hue) * (1<<16) * sat);
#endif
	while (h--) {
		for (i = 0; i<w; i++)
		{
			const int u= usrc[i] - 128;
			const int v= vsrc[i] - 128;
			int new_u= (c*u - s*v + (1<<15) + (128<<16))>>16;
			int new_v= (s*u + c*v + (1<<15) + (128<<16))>>16;
			if(new_u & 768) new_u= (-new_u)>>31;
			if(new_v & 768) new_v= (-new_v)>>31;
			udst[i]= new_u;
			vdst[i]= new_v;
		}
		usrc += srcstride;
		vsrc += srcstride;
		udst += dststride;
		vdst += dststride;
	}
}

/**
    \fn configure
*/
bool ADMVideoHue::configure()
{
    if(DIA_getHue(&_param, previousFilter))
    {
        update(&_param,&_isinus,&_icosinus);
        return true;
    }
    return false;
}
/**
    \fn getConfiguration
*/

const char   *ADMVideoHue::getConfiguration(void)
{
    static char s[256];
    snprintf(s,255," Hue :%2.2f %2.2f",_param.hue,_param.saturation);
    return s;
}
/**
    \fn ctor
*/
ADMVideoHue::ADMVideoHue(  ADM_coreVideoFilter *in,CONFcouple *couples) :
        ADM_coreVideoFilterCached(1,in,couples)
{
    if(!couples || !ADM_paramLoad(couples,hue_param,&_param))
        reset(&_param);
    update(&_param,&_isinus,&_icosinus);
}
/**
    \fn update
*/
void ADMVideoHue::update(hue *h, int *s, int *c)
{
    if(h->hue > 90.0) h->hue = 90.0;
    if(h->hue < -90.0) h->hue = -90.0;
    if(h->saturation > 10.0) h->saturation = 10.0;
    if(h->saturation < -10.0) h->saturation = -10.0;

    float fhue=h->hue * M_PI / 180.;
    float fsat=(10. + h->saturation)/10.;

    *s = (int)rint(sin(fhue) * (1<<16) * fsat);
    *c = (int)rint(cos(fhue) * (1<<16) * fsat);
}
/**
    \fn dtor
*/
ADMVideoHue::~ADMVideoHue()
{
  
}
/**
    \fn getCoupledConf
*/
bool         ADMVideoHue::getCoupledConf(CONFcouple **couples)
{
    return ADM_paramSave(couples, hue_param,&_param);
}

void ADMVideoHue::setCoupledConf(CONFcouple *couples)
{
    ADM_paramLoad(couples, hue_param, &_param);
}

/**
    \fn getNextFrame
    \brief
*/
bool         ADMVideoHue::getNextFrame(uint32_t *fn,ADMImage *image)
{
ADMImage *src;
        src=vidCache->getImage(nextFrame);
        if(!src)
            return false; // EOF
        *fn=nextFrame++;
        image->copyInfo(src);

        image->copyPlane(src,image,PLANAR_Y); // Luma is untouched

        HueProcess_C(image->GetWritePtr(PLANAR_V), image->GetWritePtr(PLANAR_U),
                     src->GetReadPtr(PLANAR_V), src->GetReadPtr(PLANAR_U),
                    
                image->GetPitch(PLANAR_U),src->GetPitch(PLANAR_U), // assume u&v pitches are =
                info.width>>1,info.height>>1, 
                _isinus, _icosinus);
 
        vidCache->unlockAll();
        return 1;
}
//EOF
