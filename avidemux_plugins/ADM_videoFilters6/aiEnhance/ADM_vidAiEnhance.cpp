/***************************************************************************
                          AiEnhance filter
        Copyright 2022 szlldm
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
#include "aiEnhance.h"
#include "aiEnhance_desc.cpp"
#include "ADM_vidAiEnhance.h"

#include "FSRCNN.h"
#include "fastFSRCNN.h"
#include "PL3NET.h"

extern uint8_t DIA_getAiEnhance(aiEnhance *param, ADM_coreVideoFilter *in);


// Add the hook to make it valid plugin
//DECLARE_VIDEO_FILTER(   ADMVideoAiEnhance,   // Class
DECLARE_VIDEO_FILTER(   ADMVideoAiEnhance,   // Class
                        1,0,0,              // Version
                        ADM_UI_TYPE_BUILD,         // UI
                        VF_TRANSFORM,            // Category
                        "aiEnhance",            // internal name (must be uniq!)
                        QT_TRANSLATE_NOOP("aiEnhance","A.I. Enhance (sw)"),            // Display name
                        QT_TRANSLATE_NOOP("aiEnhance","Neural network upscalers for low resolution videos.") // Description
                    );

/**
    \fn AiEnhanceInitializeBuffers
*/
void ADMVideoAiEnhance::AiEnhanceInitializeBuffers(int w, int h, aiEnhance_buffers_t * buffers)
{
    buffers->w = w;
    buffers->h = h;
    buffers->algo = -1; // invalidate
    buffers->ai = NULL;
    buffers->targetImg = NULL;
    buffers->chromaUpScaleImg = NULL;
    buffers->previewImg = NULL;
    buffers->upScaler = NULL;
    buffers->previewScaler = NULL;
}
/**
    \fn AiEnhanceDestroyBuffers
*/
void ADMVideoAiEnhance::AiEnhanceDestroyBuffers(aiEnhance_buffers_t * buffers)
{
    delete buffers->ai;
    delete buffers->targetImg;
    delete buffers->chromaUpScaleImg;
    delete buffers->previewImg;
    delete buffers->upScaler;
    delete buffers->previewScaler;
}

/**
    \fn AiEnhanceProcess_C
*/
void ADMVideoAiEnhance::AiEnhanceProcess_C(ADMImage *srcImg, ADMImage *dstImg, bool previewMode, int previewScale, bool skipProcess, aiEnhance param, aiEnhance_buffers_t * buffers)
{
    if (!srcImg || !dstImg || !buffers) return;

    ADM_assert(srcImg->_width == buffers->w);
    ADM_assert(srcImg->_height == buffers->h);
    ADM_assert(param.algo < 9);
    unsigned int algo = param.algo;
    
    if (buffers->algo != algo)
    {
        buffers->algo = algo;
        int scaling = getScaling(algo);
        delete buffers->ai;
        if (algo < 2)
        {
            buffers->ai = new fastFSRCNN(buffers->w, buffers->h, buffers->algo);
        }
        else
        if (algo < 8)
        {
            buffers->ai = new FSRCNN(buffers->w, buffers->h, buffers->algo - 2);
        }
        else
        {
            buffers->ai = new PL3NET(buffers->w, buffers->h, buffers->algo - 8);
        }
        delete buffers->targetImg;
        buffers->targetImg = new ADMImageDefault(buffers->w*scaling, buffers->h*scaling);
        delete buffers->chromaUpScaleImg;
        buffers->chromaUpScaleImg = new ADMImageDefault(buffers->w*scaling, buffers->h*scaling);
        delete buffers->previewImg;
        buffers->previewImg = new ADMImageDefault(buffers->w, buffers->h);
        delete buffers->upScaler;
        buffers->upScaler = new ADMColorScalerFull(ADM_CS_LANCZOS, buffers->w, buffers->h, buffers->w*scaling, buffers->h*scaling, ADM_PIXFRMT_YV12, ADM_PIXFRMT_YV12);
        delete buffers->previewScaler;
        buffers->previewScaler = new ADMColorScalerFull(ADM_CS_LANCZOS, buffers->w*scaling, buffers->h*scaling, buffers->w*previewScale, buffers->h*previewScale, ADM_PIXFRMT_YV12, ADM_PIXFRMT_YV12);
    }
    
    buffers->srcImg = srcImg;
    pthread_create( &buffers->upScalerThread, NULL, chromaUpscalerThread, (void*) buffers);
    
    if (!skipProcess)
    {
        buffers->ai->upscaleY(srcImg, buffers->targetImg);
    }

    pthread_join( buffers->upScalerThread, NULL);
    
    if (skipProcess)
        buffers->chromaUpScaleImg->copyPlane(buffers->chromaUpScaleImg, buffers->targetImg, PLANAR_Y);
    buffers->chromaUpScaleImg->copyPlane(buffers->chromaUpScaleImg, buffers->targetImg, PLANAR_U);
    buffers->chromaUpScaleImg->copyPlane(buffers->chromaUpScaleImg, buffers->targetImg, PLANAR_V);
    
    if (previewMode)
    {
        buffers->previewScaler->convertImage(buffers->targetImg, dstImg);
        dstImg->copyInfo(srcImg);
    }
    else
    {
        dstImg->duplicate(buffers->targetImg);
    }
}

void * ADMVideoAiEnhance::chromaUpscalerThread( void *ptr )
{
    aiEnhance_buffers_t * buffers = (aiEnhance_buffers_t*)ptr;
    
    buffers->upScaler->convertImage(buffers->srcImg, buffers->chromaUpScaleImg);
    
    pthread_exit(NULL);
    return NULL;   
}

/**
    \fn configure
*/
bool ADMVideoAiEnhance::configure()
{
    uint8_t r=0;

    r=  DIA_getAiEnhance(&_param, previousFilter);
    if(r) update();
    return r;
}
/**
    \fn getConfiguration
*/

const char   *ADMVideoAiEnhance::getConfiguration(void)
{
    static char s[256];
    const char * algo=NULL;
    snprintf(s,255,"[%d x %d] --> [%d x %d]", previousFilter->getInfo()->width,previousFilter->getInfo()->height, info.width,info.height);

    return s;
}
/**
    \fn ctor
*/
ADMVideoAiEnhance::ADMVideoAiEnhance(  ADM_coreVideoFilter *in,CONFcouple *couples)  :ADM_coreVideoFilter(in,couples)
{
    if(!couples || !ADM_paramLoad(couples,aiEnhance_param,&_param))
    {
        // Default value
        _param.algo = 0;
    }
    
    inputImg = new ADMImageDefault(in->getInfo()->width,in->getInfo()->height);
    AiEnhanceInitializeBuffers(in->getInfo()->width,in->getInfo()->height, &(_buffers));
    update();
}
/**
    \fn update
*/
void ADMVideoAiEnhance::update(void)
{
    int scaling = getScaling(_param.algo);
    info.width = previousFilter->getInfo()->width * scaling;
    info.height = previousFilter->getInfo()->height * scaling;
}

/**
    \fn getScaling
*/
int ADMVideoAiEnhance::getScaling(int algo)
{
    if (algo < 2)
        return fastFSRCNN::getScaling(algo);
    if (algo < 8)
        return FSRCNN::getScaling(algo - 2);
    return PL3NET::getScaling(algo - 8);
}

/**
    \fn dtor
*/
ADMVideoAiEnhance::~ADMVideoAiEnhance()
{
    delete inputImg;
    AiEnhanceDestroyBuffers(&(_buffers));
}
/**
    \fn getCoupledConf
*/
bool ADMVideoAiEnhance::getCoupledConf(CONFcouple **couples)
{
    return ADM_paramSave(couples, aiEnhance_param,&_param);
}
/**
    \fn setCoupledConf
*/
void ADMVideoAiEnhance::setCoupledConf(CONFcouple *couples)
{
    ADM_paramLoad(couples, aiEnhance_param, &_param);
}



/**
    \fn getNextFrame
    \brief
*/
bool ADMVideoAiEnhance::getNextFrame(uint32_t *fn,ADMImage *image)
{
    if(!previousFilter->getNextFrame(fn,inputImg)) return false;

    AiEnhanceProcess_C(inputImg, image, false, 1, false, _param, &(_buffers));
 
    return 1;
}
