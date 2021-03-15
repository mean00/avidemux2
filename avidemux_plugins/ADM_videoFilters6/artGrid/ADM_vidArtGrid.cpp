/***************************************************************************
                          Grid filter 
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
#include "ADM_default.h"
#include "ADM_coreVideoFilter.h"
#include "ADM_coreVideoFilterInternal.h"
#include "DIA_factory.h"
#include "artGrid.h"
#include "artGrid_desc.cpp"
#include "ADM_vidArtGrid.h"

extern uint8_t DIA_getArtGrid(artGrid *param, ADM_coreVideoFilter *in);


// Add the hook to make it valid plugin
//DECLARE_VIDEO_FILTER(   ADMVideoArtGrid,   // Class
DECLARE_VIDEO_FILTER_PARTIALIZABLE(   ADMVideoArtGrid,   // Class
                                      1,0,0,              // Version
                                      ADM_UI_TYPE_BUILD,         // UI
                                      VF_ART,            // Category
                                      "artGrid",            // internal name (must be uniq!)
                                      QT_TRANSLATE_NOOP("artGrid","Grid"),            // Display name
                                      QT_TRANSLATE_NOOP("artGrid","Video wall effect.") // Description
                                  );
/**
    \fn ArtGridProcess_C
*/
void ADMVideoArtGrid::ArtGridProcess_C(ADMImage *img, ADMImage *tmp, uint32_t size, bool roll)
{
    if (!img || !tmp) return;
    int width=img->GetWidth(PLANAR_Y); 
    int height=img->GetHeight(PLANAR_Y);
    uint8_t * plane[3], * tplane[3];
    int stride[3], tstride[3];
    int p;
    uint8_t * ptr, * iptr;
    int * line;

    if (size <= 1) return;    // nop
    if (size > 8) size = 8;
    int wg[3];
    int hg[3];
    wg[0] = (width/(size*2))*2;
    wg[2] = wg[1] = wg[0]/2;
    hg[0] = (height/(size*2))*2;
    hg[2] = hg[1] = hg[0]/2;

    line = (int*)malloc(wg[0]*sizeof(int));
    if (!line) return;

    img->GetPitches(stride);
    img->GetWritePlanes(plane);
    tmp->GetPitches(tstride);
    tmp->GetWritePlanes(tplane);

    //roll
    if (roll)
    {
        for (p=0; p<3; p++)
        {
            for (int y=(size-1); y>=0; y--)
            {
                for (int x=(size-1); x>=0; x--)
                {
                    if ((x==0) && (y==0)) break;
                    if (x==0)
                    {
                        for(int l=0; l<hg[p]; l++)
                        {
                            memcpy(&tplane[p][(y*hg[p]+l)*tstride[p] + x*wg[p]], &tplane[p][((y-1)*hg[p]+l)*tstride[p] + (size-1)*wg[p]], wg[p]);
                        }
                    } else {
                        for(int l=0; l<hg[p]; l++)
                        {
                            memcpy(&tplane[p][(y*hg[p]+l)*tstride[p] + x*wg[p]], &tplane[p][(y*hg[p]+l)*tstride[p] + (x-1)*wg[p]], wg[p]);
                        }
                    }
                }
            }
        }
    }

    // scale
    for (p=0; p<3; p++)
    {
        int xc,yc,xp,yp;
        xc=0;
        yc=0;
        xp=0;
        yp=0;
        memset(line,0,wg[p]*sizeof(int));
        for (int y=0; y<(hg[p]*size); y++)
        {
            xc=0;
            xp=0;
            for (int x=0; x<(wg[p]*size); x++)
            {
                line[xp] += plane[p][x];
                xc++;
                if (xc == size)
                {
                    xc = 0;
                    xp++;
                }
            }
            yc++;
            if (yc == size)
            {
                yc = 0;
                for (int x=0; x<wg[p]; x++)
                {
                    tplane[p][yp*tstride[p] + x] = line[x] / (size*size);
                }
                memset(line,0,wg[p]*sizeof(int));
                yp++;
            }
            plane[p] += stride[p];
        }
    }

    //duplicate
    if (roll)
    {
        uint64_t keep = img->Pts;
        ADM_colorRange range = img->_range;
        img->duplicate(tmp);
        img->Pts = keep;
        img->_range = range;
    } else {
        img->GetWritePlanes(plane);
        img->blacken();
        for (p=0; p<3; p++)
        {
            for (int y=0; y<size; y++)
            {
                for (int x=0; x<size; x++)
                {
                    for(int l=0; l<hg[p]; l++)
                    {
                        memcpy(&plane[p][(y*hg[p]+l)*stride[p] + x*wg[p]], &tplane[p][l*tstride[p]], wg[p]);
                    }
                }
            }
        }
    }

    free(line);
}

/**
    \fn configure
*/
bool ADMVideoArtGrid::configure()
{
    uint8_t r=0;

    r=  DIA_getArtGrid(&_param, previousFilter);
    if(r) update();
    return r;
}
/**
    \fn getConfiguration
*/
const char   *ADMVideoArtGrid::getConfiguration(void)
{
    static char s[256];
    snprintf(s,255,"Grid size: %dx%d, Roll previous frames: %d", _param.size, _param.size, (_param.roll ? 1:0));
    return s;
}
/**
    \fn ctor
*/
ADMVideoArtGrid::ADMVideoArtGrid(  ADM_coreVideoFilter *in,CONFcouple *couples)  :ADM_coreVideoFilter(in,couples)
{
    if(!couples || !ADM_paramLoad(couples,artGrid_param,&_param))
        reset(&_param);
    work=new ADMImageDefault(info.width,info.height);
    work->blacken();
    update();
}
/**
    \fn reset
*/
void ADMVideoArtGrid::reset(artGrid *cfg)
{
    cfg->size = 2;
    cfg->roll = false;
}
/**
    \fn valueLimit
*/
float ADMVideoArtGrid::valueLimit(float val, float min, float max)
{
    if (val < min) val = min;
    if (val > max) val = max;
    return val;
}
/**
    \fn valueLimit
*/
int32_t ADMVideoArtGrid::valueLimit(int32_t val, int32_t min, int32_t max)
{
    if (val < min) val = min;
    if (val > max) val = max;
    return val;
}
/**
    \fn update
*/
void ADMVideoArtGrid::update(void)
{
    _size=valueLimit(_param.size,1,8);
    _roll=_param.roll;
}
/**
    \fn dtor
*/
ADMVideoArtGrid::~ADMVideoArtGrid()
{
    if(work) delete work;
    work=NULL;
}
/**
    \fn getCoupledConf
*/
bool ADMVideoArtGrid::getCoupledConf(CONFcouple **couples)
{
    return ADM_paramSave(couples, artGrid_param,&_param);
}

void ADMVideoArtGrid::setCoupledConf(CONFcouple *couples)
{
    ADM_paramLoad(couples, artGrid_param, &_param);
}

/**
    \fn goToTime
*/
bool ADMVideoArtGrid::goToTime(uint64_t usec)
{
    if(_roll)
        work->blacken();
    return previousFilter->goToTime(usec);
}
/**
    \fn getNextFrame
    \brief
*/
bool ADMVideoArtGrid::getNextFrame(uint32_t *fn,ADMImage *image)
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

    ArtGridProcess_C(image, work,_size, _roll);

    return 1;
}

