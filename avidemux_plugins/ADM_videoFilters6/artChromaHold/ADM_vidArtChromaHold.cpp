/***************************************************************************
                          ChromaHold filter 
    Algorithm:
        Copyright 2021 szlldm
    Ported to Avidemux:
        Copyright 2021 szlldm
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
#include <cmath>
#include "ADM_default.h"
#include "ADM_coreVideoFilter.h"
#include "ADM_coreVideoFilterInternal.h"
#include "DIA_factory.h"
#include "artChromaHold.h"
#include "artChromaHold_desc.cpp"
#include "ADM_vidArtChromaHold.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

extern uint8_t DIA_getArtChromaHold(artChromaHold *param, ADM_coreVideoFilter *in);


// Add the hook to make it valid plugin
//DECLARE_VIDEO_FILTER(   ADMVideoArtChromaHold,   // Class
DECLARE_VIDEO_FILTER_PARTIALIZABLE(   ADMVideoArtChromaHold,   // Class
                                      1,0,0,              // Version
                                      ADM_UI_TYPE_BUILD,         // UI
                                      VF_ART,            // Category
                                      "artChromaHold",            // internal name (must be uniq!)
                                      QT_TRANSLATE_NOOP("artChromaHold","Chroma Hold"),            // Display name
                                      QT_TRANSLATE_NOOP("artChromaHold","Monochrome effect with kept color(s).") // Description
                                  );
/**
    \fn ArtChromaHoldProcess_C
*/
void ADMVideoArtChromaHold::ArtChromaHoldProcess_C(ADMImage *img, bool * cen, float * cu, float * cv, float * cdist, float * cslope)

{
    int width=img->GetWidth(PLANAR_Y); 
    int height=img->GetHeight(PLANAR_Y);
    int ustride,vstride;
    uint8_t * uptr, * vptr;
    int upixel,vpixel,pixmul;
    unsigned int * uvplane = (unsigned int *)malloc(256*256*sizeof(unsigned int));
    unsigned int uvcurr,uvnew;
    if (!uvplane) return;

    if (cen[0] || cen[1] || cen[2])
        memset(uvplane, 0, 256*256*sizeof(unsigned int));
    else
        for (int i=0; i<(256*256); i++)
            uvplane[i]=256;    // if no one enabled --> show full colors
    for (int c=0; c<3; c++)
    {
        if (!cen[c]) continue;
        int iu,iv,i,j;
        iu = std::floor(cu[c]*128.0 + 128.0);
        iv = std::floor(cv[c]*128.0 + 128.0);
        float cutdist = cdist[c]*128.0;
        float cutslope = cslope[c]*128.0;
        float diff;
        for (i=0; i<256; i++)
        {
            for (j=0; j<256; j++)
            {
                diff = std::sqrt(((iu-i)*(iu-i) + (iv-j)*(iv-j))) - cutdist;
                if (diff <= 0) uvplane[i*256+j] = 256;
                else {
                    uvcurr = uvplane[i*256+j];
                    if ((diff > cutslope) || (cutslope == 0.0)) uvnew = 0;
                    else {
                        // diff == 0 -> 256 ...... diff == cutslope --> 0
                        uvnew = std::floor(256.0 - 256.0*valueLimit((diff / cutslope), 0.0, 1.0));
                    }

                    if (uvnew > uvcurr) uvplane[i*256+j] = uvnew;
                }
            }
        }
    }

    if(img->_range == ADM_COL_RANGE_MPEG)
        img->expandColorRange();

    // Y plane - no change

    // UV planes
    ustride=img->GetPitch(PLANAR_U);
    uptr=img->GetWritePtr(PLANAR_U);
    vstride=img->GetPitch(PLANAR_V);
    vptr=img->GetWritePtr(PLANAR_V);
    for(int y=0;y<height/2;y++)	// 4:2:0
    {
        for (int x=0;x<width/2;x++)
        {
            upixel = uptr[x];
            vpixel = vptr[x];
            pixmul = uvplane[upixel*256 + vpixel];
            upixel -= 128;
            vpixel -= 128;
            upixel = (upixel * pixmul) >> 8;
            vpixel = (vpixel * pixmul) >> 8;
            uptr[x] = upixel + 128;
            vptr[x] = vpixel + 128;
        }
        uptr+=ustride;
        vptr+=vstride;
    }

    free(uvplane);
}

/**
    \fn configure
*/
bool ADMVideoArtChromaHold::configure()
{
    uint8_t r=0;

    r=  DIA_getArtChromaHold(&_param, previousFilter);
    if(r) update();
    return r;
}
/**
    \fn getConfiguration
*/
const char   *ADMVideoArtChromaHold::getConfiguration(void)
{
    static char s[256];
    static char s1[64],s2[64],s3[64];

    if (_param.c1en)
    {
        snprintf(s1,63,"[%.2f,%.2f]:{r=%.2f,s=%.2f}, ",_param.c1u,_param.c1v,_param.c1dist,_param.c1slope);
    } else {
        strcpy(s1,"");
    }

    if (_param.c2en)
    {
        snprintf(s2,63,"[%.2f,%.2f]:{r=%.2f,s=%.2f}, ",_param.c2u,_param.c2v,_param.c2dist,_param.c2slope);
    } else {
        strcpy(s2,"");
    }

    if (_param.c3en)
    {
        snprintf(s3,63,"[%.2f,%.2f]:{r=%.2f,s=%.2f}, ",_param.c3u,_param.c3v,_param.c3dist,_param.c3slope);
    } else {
        strcpy(s3,"");
    }
    snprintf(s,255," Chroma hold: %s%s%s",s1,s2,s3);

    return s;
}
/**
    \fn ctor
*/
ADMVideoArtChromaHold::ADMVideoArtChromaHold(  ADM_coreVideoFilter *in,CONFcouple *couples)  :ADM_coreVideoFilter(in,couples)
{
    if(!couples || !ADM_paramLoad(couples,artChromaHold_param,&_param))
    {
        _param.c1en = true;
        _param.c1u = 0.0;
        _param.c1v = 0.0;
        _param.c1dist = 0.5;
        _param.c1slope = 0.0;
        _param.c2en =false;
        _param.c2u = 0.0;
        _param.c2v = 0.0;
        _param.c2dist = 0.0;
        _param.c2slope = 0.0;
        _param.c3en =false;
        _param.c3u = 0.0;
        _param.c3v = 0.0;
        _param.c3dist = 0.0;
        _param.c3slope = 0.0;
    }
    update();
}
/**
    \fn valueLimit
*/
float ADMVideoArtChromaHold::valueLimit(float val, float min, float max)
{
    if (val < min) val = min;
    if (val > max) val = max;
    return val;
}
/**
    \fn valueLimit
*/
int32_t ADMVideoArtChromaHold::valueLimit(int32_t val, int32_t min, int32_t max)
{
    if (val < min) val = min;
    if (val > max) val = max;
    return val;
}
/**
    \fn update
*/
void ADMVideoArtChromaHold::update(void)
{
    _cen[0] = _param.c1en;
    _cen[1] = _param.c2en;
    _cen[2] = _param.c3en;
    _cu[0] = _param.c1u;
    _cu[1] = _param.c2u;
    _cu[2] = _param.c3u;
    _cv[0] = _param.c1v;
    _cv[1] = _param.c2v;
    _cv[2] = _param.c3v;
    _cdist[0] = _param.c1dist;
    _cdist[1] = _param.c2dist;
    _cdist[2] = _param.c3dist;
    _cslope[0] = _param.c1slope;
    _cslope[1] = _param.c2slope;
    _cslope[2] = _param.c3slope;

    for (int i=0; i<3; i++)
    {
        _cu[i] = valueLimit(_cu[i], -1.0, 1.0);
        _cv[i] = valueLimit(_cv[i], -1.0, 1.0);
        _cdist[i] = valueLimit(_cdist[i], 0.0, 1.0);
        _cslope[i] = valueLimit(_cslope[i], 0.0, 1.0);
    }

}
/**
    \fn dtor
*/
ADMVideoArtChromaHold::~ADMVideoArtChromaHold()
{
    
}
/**
    \fn getCoupledConf
*/
bool ADMVideoArtChromaHold::getCoupledConf(CONFcouple **couples)
{
    return ADM_paramSave(couples, artChromaHold_param,&_param);
}

void ADMVideoArtChromaHold::setCoupledConf(CONFcouple *couples)
{
    ADM_paramLoad(couples, artChromaHold_param, &_param);
}

/**
    
*/

/**
    \fn getNextFrame
    \brief
*/
bool ADMVideoArtChromaHold::getNextFrame(uint32_t *fn,ADMImage *image)
{
    /*
    ADMImage *src;
    src=vidCache->getImage(nextFrame);
    if(!src)
        return false; // EOF
    *fn=nextFrame++;
    image->copyInfo(src);
    image->copyPlane(src,image,PLANAR_Y); // Luma is untouched
    src = image;

    DoFilter(...);

    vidCache->unlockAll();
    */
    if(!previousFilter->getNextFrame(fn,image)) return false;

    ArtChromaHoldProcess_C(image, _cen, _cu, _cv, _cdist, _cslope);

    return 1;
}

