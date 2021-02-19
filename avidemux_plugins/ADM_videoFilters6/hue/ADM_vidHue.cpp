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
    \fn     buildLut
    \brief  Populate lookup tables
*/
static void buildLut(uint8_t utab[][256], uint8_t vtab[][256], const int s, const int c)
{
    int i,j,u,v,uout,vout;
    for(i=0; i<256; i++)
    {
        for(j=0; j<256; j++)
        {
            u = i-128;
            v = j-128;
            uout = (c*u - s*v + (1 << 15) + (128 << 16)) >> 16;
            vout = (s*u + c*v + (1 << 15) + (128 << 16)) >> 16;
            if(uout & (~0xFF)) uout = (~uout) >> 31;
            if(vout & (~0xFF)) vout = (~vout) >> 31;

            utab[i][j] = uout;
            vtab[i][j] = vout;
        }
    }
}
/**
    \fn boundCheck
*/
static void boundCheck(float &h, float &s)
{
    if(h > 90.) h = 90.;
    if(h < -90.) h = -90.;
    if(s > 10.) s = 10.;
    if(s < -10.) s = -10.;
}

/**
    \fn HueProcess_C
*/
void ADMVideoHue::HueProcess_C(uint8_t *udst, uint8_t *vdst, uint8_t *usrc, uint8_t *vsrc,
                               int dststride, int srcstride, int w, int h, huesettings *s)
{
    int i;

    while(h--)
    {
        for (i = 0; i < w; i++)
        {
            const int u = usrc[i];
            const int v = vsrc[i];
            udst[i] = s->lutU[u][v];
            vdst[i] = s->lutV[u][v];
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
    if(DIA_getHue(&_settings.param, previousFilter))
    {
        update(&_settings);
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
    snprintf(s,255," Hue :%2.2f %2.2f",_settings.param.hue,_settings.param.saturation);
    return s;
}
/**
    \fn ctor
*/
ADMVideoHue::ADMVideoHue(  ADM_coreVideoFilter *in,CONFcouple *couples) :
        ADM_coreVideoFilterCached(1,in,couples)
{
    if(!couples || !ADM_paramLoad(couples,hue_param,&_settings.param))
        reset(&_settings.param);
    update(&_settings);
}
/**
    \fn update
*/
void ADMVideoHue::update(huesettings *sts)
{
    boundCheck(sts->param.hue,sts->param.saturation);

    float hue = sts->param.hue * M_PI / 180.;
    float sat = (10. + sts->param.saturation)/10.;

    int s = (int)rint(sin(hue) * (1<<16) * sat);
    int c = (int)rint(cos(hue) * (1<<16) * sat);

    buildLut(sts->lutU,sts->lutV,s,c);
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
    return ADM_paramSave(couples, hue_param, &_settings.param);
}

void ADMVideoHue::setCoupledConf(CONFcouple *couples)
{
    ADM_paramLoad(couples, hue_param, &_settings.param);
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

        HueProcess_C(image->GetWritePtr(PLANAR_U), image->GetWritePtr(PLANAR_V),
                     src->GetReadPtr(PLANAR_U), src->GetReadPtr(PLANAR_V),
                     image->GetPitch(PLANAR_U), src->GetPitch(PLANAR_U), // assume u&v pitches are =
                     info.width>>1, info.height>>1, &_settings);

        vidCache->unlockAll();
        return 1;
}
//EOF
