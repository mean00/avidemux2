/***************************************************************************
                          Pixelize filter 

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
#include "artPixelize.h"
#include "artPixelize_desc.cpp"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

extern uint8_t DIA_getArtPixelize(artPixelize *param, ADM_coreVideoFilter *in);
/**
    \class ADMVideoArtPixelize
*/
class  ADMVideoArtPixelize:public ADM_coreVideoFilter
{

  protected:
    void            update(void);
    artPixelize       _param;
    unsigned int      _pw, _ph;
  public:
    ADMVideoArtPixelize(ADM_coreVideoFilter *in,CONFcouple *couples);
    ~ADMVideoArtPixelize();

    virtual const char   *getConfiguration(void);          /// Return  current configuration as a human readable string
    virtual bool         getNextFrame(uint32_t *fn,ADMImage *image);    /// Return the next image
    virtual bool         getCoupledConf(CONFcouple **couples) ;   /// Return the current filter configuration
    virtual void         setCoupledConf(CONFcouple *couples);
    virtual bool         configure(void) ;                 /// Start graphical user interface

};



// Add the hook to make it valid plugin
//DECLARE_VIDEO_FILTER(   ADMVideoArtPixelize,   // Class
DECLARE_VIDEO_FILTER_PARTIALIZABLE(   ADMVideoArtPixelize,   // Class
                                      1,0,0,              // Version
                                      ADM_UI_TYPE_BUILD,         // UI
                                      VF_ART,            // Category
                                      "artPixelize",            // internal name (must be uniq!)
                                      QT_TRANSLATE_NOOP("artPixelize","Pixelize"),            // Display name
                                      QT_TRANSLATE_NOOP("artPixelize","Pixelize image.") // Description
                                  );
/**
    \fn ArtPixelizeCore_C
*/
void ArtPixelizeCore_C(uint8_t * ptr, unsigned int stride, unsigned int width, unsigned int height, unsigned int pw, unsigned int ph)
{
    uint8_t * ptrp;
    float pixel;
    uint8_t pixelU8;
    float sumcnt;

    for(int y=0;y<height;y+=ph)
    {
        for (int x=0;x<width;x+=pw)
        {
            pixel = 0;
            sumcnt = 0;
            ptrp = &ptr[x];
            for (int j=0; j<ph; j++)
            {
                if (y+j >= height) break;
                for (int i=0; i<pw; i++)
                {
                    if (x+i >= width) break;
                    pixel += ptrp[i];
                    sumcnt += 1;
                }
                ptrp += stride;
            }

            if (sumcnt > 0) {
                pixel /= sumcnt;
            } else {
                pixel = 0;
            }

            pixelU8 = (uint8_t)std::round(pixel);

            ptrp = &ptr[x];
            for (int j=0; j<ph; j++)
            {
                if (y+j >= height) break;
                for (int i=0; i<pw; i++)
                {
                    if (x+i >= width) break;
                    ptrp[i] = pixelU8;
                }
                ptrp += stride;
            }
        }
        ptr+=stride*ph;
    }
}
/**
    \fn ArtPixelizeProcess_C
*/
void ArtPixelizeProcess_C(ADMImage *img, unsigned int pw, unsigned int ph)
{
    int width=img->GetWidth(PLANAR_Y); 
    int height=img->GetHeight(PLANAR_Y);
    int stride;
    uint8_t * ptr;

    pw = pw & 0xFFFE;	// must be even
    ph = ph & 0xFFFE;	// must be even

    // Y plane
    stride=img->GetPitch(PLANAR_Y);
    ptr=img->GetWritePtr(PLANAR_Y);
    ArtPixelizeCore_C(ptr, stride, width, height, pw, ph);
    // UV planes
    for (int p=1; p<3; p++)
    {
        stride=img->GetPitch((ADM_PLANE)p);
        ptr=img->GetWritePtr((ADM_PLANE)p);
        ArtPixelizeCore_C(ptr, stride, width/2, height/2, pw/2, ph/2);	// 4:2:0
    }
}

/**
    \fn configure
*/
bool ADMVideoArtPixelize::configure()
{
    uint8_t r=0;

    r=  DIA_getArtPixelize(&_param, previousFilter);
    if(r) update();
    return r;
}
/**
    \fn getConfiguration
*/
const char   *ADMVideoArtPixelize::getConfiguration(void)
{
    static char s[256];
    snprintf(s,255," Pixel size: %ux%u",_param.pw, _param.ph);
    return s;
}
/**
    \fn ctor
*/
ADMVideoArtPixelize::ADMVideoArtPixelize(  ADM_coreVideoFilter *in,CONFcouple *couples)  :ADM_coreVideoFilter(in,couples)
{
    if(!couples || !ADM_paramLoad(couples,artPixelize_param,&_param))
    {
        _param.pw = 2;
        _param.ph = 2;
    }
    update();
}
/**
    \fn update
*/
void ADMVideoArtPixelize::update(void)
{
    _pw=_param.pw;
    _ph=_param.ph;
}
/**
    \fn dtor
*/
ADMVideoArtPixelize::~ADMVideoArtPixelize()
{
    
}
/**
    \fn getCoupledConf
*/
bool ADMVideoArtPixelize::getCoupledConf(CONFcouple **couples)
{
    return ADM_paramSave(couples, artPixelize_param,&_param);
}

void ADMVideoArtPixelize::setCoupledConf(CONFcouple *couples)
{
    ADM_paramLoad(couples, artPixelize_param, &_param);
}

/**
    
*/

/**
    \fn getNextFrame
    \brief
*/
bool ADMVideoArtPixelize::getNextFrame(uint32_t *fn,ADMImage *image)
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

    ArtPixelizeProcess_C(image, _pw, _ph);

    return 1;
}

