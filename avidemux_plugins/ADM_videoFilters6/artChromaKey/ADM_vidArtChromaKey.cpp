/***************************************************************************
                          ChromaKey filter 
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
#include "artChromaKey.h"
#include "artChromaKey_desc.cpp"
#include "ADM_vidArtChromaKey.h"
#include "ADM_imageLoader.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

extern uint8_t DIA_getArtChromaKey(artChromaKey *param, ADM_coreVideoFilter *in);


// Add the hook to make it valid plugin
//DECLARE_VIDEO_FILTER(   ADMVideoArtChromaKey,   // Class
DECLARE_VIDEO_FILTER_PARTIALIZABLE(   ADMVideoArtChromaKey,   // Class
                                      1,0,0,              // Version
                                      ADM_UI_TYPE_BUILD,         // UI
                                      VF_ART,            // Category
                                      "artChromaKey",            // internal name (must be uniq!)
                                      QT_TRANSLATE_NOOP("artChromaKey","Chroma Key"),            // Display name
                                      QT_TRANSLATE_NOOP("artChromaKey","Replace \"green screen\" with an image.") // Description
                                  );
/**
    \fn ArtChromaKeyProcess_C
*/
void ADMVideoArtChromaKey::ArtChromaKeyProcess_C(ADMImage *img, ADMImage *backgrnd, bool * cen, float * cu, float * cv, float * cdist, float * cslope, uint32_t spill)
{
    if (!backgrnd) return;
    int width=img->GetWidth(PLANAR_Y); 
    int height=img->GetHeight(PLANAR_Y);
    int iystride, iustride,ivstride;
    uint8_t * iyptr, * iuptr, * ivptr;
    int bystride, bustride,bvstride;
    uint8_t * byptr, * buptr, * bvptr;
    int upixel,vpixel,pixmul,pixel,pixsum;

    unsigned int * uvplane = (unsigned int *)malloc(256*256*sizeof(unsigned int));
    int uvcurr,uvnew;
    if (!uvplane) return;

    if(img->_range == ADM_COL_RANGE_MPEG)
        img->expandColorRange();

    unsigned int * alphaneg = NULL;
    if (spill)
    {
        alphaneg = (unsigned int *)malloc(width*height*sizeof(unsigned int));
        if (!alphaneg)
            spill = 0;
    }

    memset(uvplane, 0, 256*256*sizeof(unsigned int));
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

    // YUV planes
    iystride=img->GetPitch(PLANAR_Y);
    iyptr=img->GetWritePtr(PLANAR_Y);
    iustride=img->GetPitch(PLANAR_U);
    iuptr=img->GetWritePtr(PLANAR_U);
    ivstride=img->GetPitch(PLANAR_V);
    ivptr=img->GetWritePtr(PLANAR_V);
    bystride=backgrnd->GetPitch(PLANAR_Y);
    byptr=backgrnd->GetWritePtr(PLANAR_Y);
    bustride=backgrnd->GetPitch(PLANAR_U);
    buptr=backgrnd->GetWritePtr(PLANAR_U);
    bvstride=backgrnd->GetPitch(PLANAR_V);
    bvptr=backgrnd->GetWritePtr(PLANAR_V);
    int x,y;

    for(y=0;y<height;y++)
    {
        for (x=0;x<width;x++)
        {
            upixel = iuptr[x/2];
            vpixel = ivptr[x/2];
            pixmul = uvplane[upixel*256 + vpixel];

            if ( spill && ((x/2) > 0) && ((x/2) < (width/2 -1))  && ((y/2) > 0) && ((y/2) < (height/2 -1)) )
            {
                int quadrant = (((height&1) << 1) + (width&1))&3;
                int pixmuls[4];
                pixmuls[0] = pixmul;
                switch(quadrant)
                {
                    case 0:    // left, up
                            upixel = iuptr[x/2-1];
                            vpixel = ivptr[x/2-1];
                            pixmuls[1] = uvplane[upixel*256 + vpixel];
                            upixel = (iuptr-iustride)[x/2];
                            vpixel = (ivptr-ivstride)[x/2];
                            pixmuls[2] = uvplane[upixel*256 + vpixel];
                            upixel = (iuptr-iustride)[x/2-1];
                            vpixel = (ivptr-ivstride)[x/2-1];
                            pixmuls[3] = uvplane[upixel*256 + vpixel];
                        break;
                    case 1:    // right, up
                            upixel = iuptr[x/2+1];
                            vpixel = ivptr[x/2+1];
                            pixmuls[1] = uvplane[upixel*256 + vpixel];
                            upixel = (iuptr-iustride)[x/2];
                            vpixel = (ivptr-ivstride)[x/2];
                            pixmuls[2] = uvplane[upixel*256 + vpixel];
                            upixel = (iuptr-iustride)[x/2+1];
                            vpixel = (ivptr-ivstride)[x/2+1];
                            pixmuls[3] = uvplane[upixel*256 + vpixel];
                        break;
                    case 2:    // left, down
                            upixel = iuptr[x/2-1];
                            vpixel = ivptr[x/2-1];
                            pixmuls[1] = uvplane[upixel*256 + vpixel];
                            upixel = (iuptr+iustride)[x/2];
                            vpixel = (ivptr+ivstride)[x/2];
                            pixmuls[2] = uvplane[upixel*256 + vpixel];
                            upixel = (iuptr+iustride)[x/2-1];
                            vpixel = (ivptr+ivstride)[x/2-1];
                            pixmuls[3] = uvplane[upixel*256 + vpixel];
                        break;
                    default:
                    case 3:    // right, down
                            upixel = iuptr[x/2+1];
                            vpixel = ivptr[x/2+1];
                            pixmuls[1] = uvplane[upixel*256 + vpixel];
                            upixel = (iuptr+iustride)[x/2];
                            vpixel = (ivptr+ivstride)[x/2];
                            pixmuls[2] = uvplane[upixel*256 + vpixel];
                            upixel = (iuptr+iustride)[x/2+1];
                            vpixel = (ivptr+ivstride)[x/2+1];
                            pixmuls[3] = uvplane[upixel*256 + vpixel];
                        break;
                }

                switch(spill)
                {
                    case 1:    // Weighted alpha
                            pixmul = (9*pixmuls[0] + 3*pixmuls[1] + 3*pixmuls[2] + 1*pixmuls[3])/16;
                        break;
                    case 2:    // AVG alpha
                            pixmul = 0;
                            for (int i=0; i<4; i++)
                            {
                                pixmul += pixmuls[i];
                            }
                            pixmul /= 4;
                        break;
                    case 3:    // RMS alpha
                            pixmul = 0;
                            for (int i=0; i<4; i++)
                            {
                                pixmul += pixmuls[i]*pixmuls[i];
                            }
                            pixmul /= 4;
                            pixmul = (int)std::sqrt(pixmul);
                        break;
                    case 4:    // MIN alpha
                            pixmul = pixmuls[0];
                            for (int i=1; i<4; i++)
                            {
                                if (pixmul < pixmuls[i]) pixmul = pixmuls[i];
                            }
                        break;
                    default:
                        break;
                }

            }

            if ( spill)
            {
                alphaneg[y*width+x] = pixmul;
            }

            if (pixmul <= 0) continue;    // video frame pixel
            if (pixmul >= 256)    // background image pixel
            {
                iyptr[x]   = byptr[x];
                continue;
            }
            // blending:
            pixel = iyptr[x];
            pixsum = pixel * (256 - pixmul);
            pixel = byptr[x];
            pixsum += pixel * pixmul;
            iyptr[x] = pixsum >> 8;
        }

        iyptr += iystride;
        byptr += bystride;
        if (y%2)	// 4:2:0
        {
            iuptr+=iustride;
            ivptr+=ivstride;
        }
    }

    iuptr=img->GetWritePtr(PLANAR_U);
    ivptr=img->GetWritePtr(PLANAR_V);
    buptr=backgrnd->GetWritePtr(PLANAR_U);
    bvptr=backgrnd->GetWritePtr(PLANAR_V);
    for(y=0;y<height/2;y++)
    {
        for (x=0;x<width/2;x++)
        {
            upixel = iuptr[x];
            vpixel = ivptr[x];
            pixmul = uvplane[upixel*256 + vpixel];

            if ( spill )
            {
                int pixmuls[4];
                pixmuls[0] = alphaneg[(2*y    )*width+(2*x    )];
                pixmuls[1] = alphaneg[(2*y    )*width+(2*x + 1)];
                pixmuls[2] = alphaneg[(2*y + 1)*width+(2*x    )];
                pixmuls[3] = alphaneg[(2*y + 1)*width+(2*x + 1)];

                switch(spill)
                {
                    case 1:
                    case 2:    // Weighted alpha / AVG alpha
                            pixmul = 0;
                            for (int i=0; i<4; i++)
                            {
                                pixmul += pixmuls[i];
                            }
                            pixmul /= 4;
                        break;
                    case 3:    // RMS alpha
                            pixmul = 0;
                            for (int i=0; i<4; i++)
                            {
                                pixmul += pixmuls[i]*pixmuls[i];
                            }
                            pixmul /= 4;
                            pixmul = (int)std::sqrt(pixmul);
                        break;
                    case 4:    // MIN alpha
                            pixmul = pixmuls[0];
                            for (int i=1; i<4; i++)
                            {
                                if (pixmul < pixmuls[i]) pixmul = pixmuls[i];
                            }
                        break;

                    default:
                        break;
                }

            }

            if (pixmul <= 0) continue;    // video frame pixel
            if (pixmul >= 256)    // background image pixel
            {
                iuptr[x] = buptr[x];
                ivptr[x] = bvptr[x];
                continue;
            }
            // blending:
            pixel = iuptr[x];
            pixsum = pixel * (256 - pixmul);
            pixel = buptr[x];
            pixsum += pixel * pixmul;
            iuptr[x] = pixsum >> 8;
            pixel = ivptr[x];
            pixsum = pixel * (256 - pixmul);
            pixel = bvptr[x];
            pixsum += pixel * pixmul;
            ivptr[x] = pixsum >> 8;
        }
        iuptr+=iustride;
        ivptr+=ivstride;
        buptr+=bustride;
        bvptr+=bvstride;
    }

    free(uvplane);
    free(alphaneg);
}

/**
    \fn configure
*/
bool ADMVideoArtChromaKey::configure()
{
    uint8_t r=0;

    r=  DIA_getArtChromaKey(&_param, previousFilter);
    reloadImage();
    if(r) update();
    return r;
}
/**
    \fn getConfiguration
*/
const char   *ADMVideoArtChromaKey::getConfiguration(void)
{
    static char s[2560];
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
    snprintf(s,2559," Chroma key: %s%s%s File:%s, Spill control: %d",s1,s2,s3,_param.imagefile.c_str(),_param.spill);


    return s;
}
/**
    \fn ctor
*/
ADMVideoArtChromaKey::ADMVideoArtChromaKey(  ADM_coreVideoFilter *in,CONFcouple *couples)  :ADM_coreVideoFilter(in,couples)
{
    if(!couples || !ADM_paramLoad(couples,artChromaKey_param,&_param))
    {
        _param.c1en = false;
        _param.c1u = 0.0;
        _param.c1v = 0.0;
        _param.c1dist = 0.0;
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
        _param.spill = 0;
    }
    _backgrnd=new ADMImageDefault(info.width,info.height);
    _backgrnd->blacken();
    if(_backgrnd->_range == ADM_COL_RANGE_MPEG)
        _backgrnd->expandColorRange();
    reloadImage();
    update();
}
/**
    \fn reloadImage
*/
bool ADMVideoArtChromaKey::reloadImage(void)
{
        if(!_param.imagefile.size())
        {
            return false;
        }
        ADMImage *im2=createImageFromFile(_param.imagefile.c_str());
        if(!im2) return false;
        ADMColorScalerFull * scaler = new ADMColorScalerFull(ADM_CS_BICUBIC,im2->GetWidth(PLANAR_Y),im2->GetHeight(PLANAR_Y),_backgrnd->GetWidth(PLANAR_Y),_backgrnd->GetHeight(PLANAR_Y),im2->_pixfrmt,ADM_PIXFRMT_YV12);

        if (scaler) {
            scaler->convertImage(im2, _backgrnd);
            if(_backgrnd->_range == ADM_COL_RANGE_MPEG)
                _backgrnd->expandColorRange();
            delete scaler;
        }

        delete im2;

        return true;
}
/**
    \fn valueLimit
*/
float ADMVideoArtChromaKey::valueLimit(float val, float min, float max)
{
    if (val < min) val = min;
    if (val > max) val = max;
    return val;
}
/**
    \fn valueLimit
*/
int32_t ADMVideoArtChromaKey::valueLimit(int32_t val, int32_t min, int32_t max)
{
    if (val < min) val = min;
    if (val > max) val = max;
    return val;
}
/**
    \fn update
*/
void ADMVideoArtChromaKey::update(void)
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
    _spill = _param.spill;

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
ADMVideoArtChromaKey::~ADMVideoArtChromaKey()
{
    if(_backgrnd) delete _backgrnd;
}
/**
    \fn getCoupledConf
*/
bool ADMVideoArtChromaKey::getCoupledConf(CONFcouple **couples)
{
    return ADM_paramSave(couples, artChromaKey_param,&_param);
}

void ADMVideoArtChromaKey::setCoupledConf(CONFcouple *couples)
{
    ADM_paramLoad(couples, artChromaKey_param, &_param);
}

/**
    
*/

/**
    \fn getNextFrame
    \brief
*/
bool ADMVideoArtChromaKey::getNextFrame(uint32_t *fn,ADMImage *image)
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

    ArtChromaKeyProcess_C(image, _backgrnd, _cen, _cu, _cv, _cdist, _cslope, _spill);

    return 1;
}

