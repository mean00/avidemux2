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



extern uint8_t DIA_getAiEnhance(aiEnhance *param, ADM_coreVideoFilter *in);


// Add the hook to make it valid plugin
//DECLARE_VIDEO_FILTER(   ADMVideoAiEnhance,   // Class
DECLARE_VIDEO_FILTER(   ADMVideoAiEnhance,   // Class
                        1,0,0,              // Version
                        ADM_UI_TYPE_BUILD,         // UI
                        VF_TRANSFORM,            // Category
                        "aiEnhance",            // internal name (must be uniq!)
                        QT_TRANSLATE_NOOP("aiEnhance","A.I. Enhace (sw)"),            // Display name
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
    buffers->previewImg = NULL;
    buffers->upScaler = NULL;
    buffers->previewScaler = NULL;
    
    buffers->threads = ADM_cpu_num_processors();
    if (buffers->threads < 1)
        buffers->threads = 1;
    if (buffers->threads > 64)
        buffers->threads = 64;
}
/**
    \fn AiEnhanceDestroyBuffers
*/
void ADMVideoAiEnhance::AiEnhanceDestroyBuffers(aiEnhance_buffers_t * buffers)
{
    delete buffers->ai;
    delete buffers->targetImg;
    delete buffers->previewImg;
    delete buffers->upScaler;
    delete buffers->previewScaler;
}

/**
    \fn AiEnhanceProcess_C
*/
void ADMVideoAiEnhance::AiEnhanceProcess_C(ADMImage *srcImg, ADMImage *dstImg, bool previewMode, bool skipProcess, aiEnhance param, aiEnhance_buffers_t * buffers)
{
    if (!srcImg || !dstImg || !buffers) return;

    ADM_assert(srcImg->_width == buffers->w);
    ADM_assert(srcImg->_height == buffers->h);
    ADM_assert(param.algo < 6);
    unsigned int algo = param.algo;
    
    if (buffers->algo != algo)
    {
        buffers->algo = algo;
        int scaling = FSRCNN::getScaling(algo);
        delete buffers->ai;
        buffers->ai = new FSRCNN(buffers->w, buffers->h, buffers->algo, buffers->threads);
        delete buffers->targetImg;
        buffers->targetImg = new ADMImageDefault(buffers->w*scaling, buffers->h*scaling);
        delete buffers->previewImg;
        buffers->previewImg = new ADMImageDefault(buffers->w, buffers->h);
        delete buffers->upScaler;
        buffers->upScaler = new ADMColorScalerFull(ADM_CS_LANCZOS, buffers->w, buffers->h, buffers->w*scaling, buffers->h*scaling, ADM_PIXFRMT_YV12, ADM_PIXFRMT_YV12);
        delete buffers->previewScaler;
        buffers->previewScaler = new ADMColorScalerFull(ADM_CS_LANCZOS, buffers->w*scaling, buffers->h*scaling, buffers->w*2, buffers->h*2, ADM_PIXFRMT_YV12, ADM_PIXFRMT_YV12);
    }
    
    buffers->upScaler->convertImage(srcImg, buffers->targetImg);
    
    if (!skipProcess)
    {
        buffers->ai->upscaleY(srcImg, buffers->targetImg);
    }
    
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
        _param.algo = 3;
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
    int scaling = FSRCNN::getScaling(_param.algo);
    info.width = previousFilter->getInfo()->width * scaling;
    info.height = previousFilter->getInfo()->height * scaling;
}

/**
    \fn getScaling
*/
int ADMVideoAiEnhance::getScaling(aiEnhance param)
{
    return FSRCNN::getScaling(param.algo);
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

    AiEnhanceProcess_C(inputImg, image, false, false, _param, &(_buffers));
 
    return 1;
}




int FSRCNN::getScaling(int algo)
{
    int scaling = -1;
    switch(algo) {
        case 0:
        case 1:
        case 2:
        case 3:
            scaling = 2;
            break;
        case 4:
            scaling = 3;
            break;
        case 5:
            scaling = 4;
            break;
        default:
            ADM_assert(0);
            break;
    }
    return scaling;
}

#include "fx2a.h"
#include "fx2d.h"
#include "fx2.h"
#include "fx3.h"
#include "fx4.h"


FSRCNN::FSRCNN(int w, int h, int algo, int threads)
{
    this->w = w;
    this->h = h;
    paddedImg = new ADMImageDefault(w+4, h+4);
    paddedImgPlane = paddedImg->GetReadPtr(PLANAR_Y);
    paddedImgStride = paddedImg->GetPitch(PLANAR_Y);    
    
    scaling = FSRCNN::getScaling(algo);
    if (threads < 1) threads = 1;
    this->threads = threads;
    
    worker_threads = new pthread_t[threads];
    worker_thread_args = new worker_thread_arg[threads];
    
    
    layerFeature = (float*)ADM_alloc( (w+2)*(h+2)*16* sizeof(float));    // must be aligned
    memset(layerFeature, 0,  (w+2)*(h+2)*16* sizeof(float));
    layerModel1 = (float*)ADM_alloc( (w+2)*(h+2)*16* sizeof(float));    // must be aligned
    memset(layerModel1, 0,  (w+2)*(h+2)*16* sizeof(float));
    layerModel2 = (float*)ADM_alloc( (w+2)*(h+2)*16* sizeof(float));    // must be aligned
    memset(layerModel2, 0,  (w+2)*(h+2)*16* sizeof(float));
    layerModel3 = (float*)ADM_alloc( (w+2)*(h+2)*16* sizeof(float));    // must be aligned
    memset(layerModel3, 0,  (w+2)*(h+2)*16* sizeof(float));
    layerModel4 = (float*)ADM_alloc( (w+2)*(h+2)*16* sizeof(float));    // must be aligned
    memset(layerModel4, 0,  (w+2)*(h+2)*16* sizeof(float));
    layerResidual = (float*)ADM_alloc( (w+2)*(h+2)*16* sizeof(float));    // must be aligned
    memset(layerResidual, 0,  (w+2)*(h+2)*16* sizeof(float));
    
    paramFeatureBias = (float*)ADM_alloc(16 * sizeof(float));    // must be aligned
    paramFeatureWeights = (float*)ADM_alloc(25 * 4 * 4 * sizeof(float));    // must be aligned
    paramModel1Bias = (float*)ADM_alloc(16 * sizeof(float));    // must be aligned
    paramModel1Weights = (float*)ADM_alloc(36 * 16 * 4 * sizeof(float));    // must be aligned
    paramModel1Alpha = (float*)ADM_alloc(16 * sizeof(float));    // must be aligned
    paramModel2Bias = (float*)ADM_alloc(16 * sizeof(float));    // must be aligned
    paramModel2Weights = (float*)ADM_alloc(36 * 16 * 4 * sizeof(float));    // must be aligned
    paramModel2Alpha = (float*)ADM_alloc(16 * sizeof(float));    // must be aligned
    paramModel3Bias = (float*)ADM_alloc(16 * sizeof(float));    // must be aligned
    paramModel3Weights = (float*)ADM_alloc(36 * 16 * 4 * sizeof(float));    // must be aligned
    paramModel3Alpha = (float*)ADM_alloc(16 * sizeof(float));    // must be aligned
    paramModel4Bias = (float*)ADM_alloc(16 * sizeof(float));    // must be aligned
    paramModel4Weights = (float*)ADM_alloc(36 * 16 * 4 * sizeof(float));    // must be aligned
    paramModel4Alpha = (float*)ADM_alloc(16 * sizeof(float));    // must be aligned
    paramResidualBias = (float*)ADM_alloc(16 * sizeof(float));    // must be aligned
    paramResidualWeights = (float*)ADM_alloc(16 * 4 * 4 * sizeof(float));    // must be aligned
    paramResidualAlpha = (float*)ADM_alloc(16 * sizeof(float));    // must be aligned
    paramSubConvolutionBias = (float*)ADM_alloc(4*scaling * sizeof(float));    // must be aligned
    paramSubConvolutionWeights = (float*)ADM_alloc(36 * 16 * scaling * sizeof(float));    // must be aligned
    
    switch(algo) {
        case 0:
            memcpy(paramFeatureBias, fx2a_paramFeatureBias, 16 * sizeof(float));
            memcpy(paramFeatureWeights, fx2a_paramFeatureWeights, 25 * 4 * 4 * sizeof(float));
            memcpy(paramModel1Bias, fx2a_paramModel1Bias, 16 * sizeof(float));
            memcpy(paramModel1Weights, fx2a_paramModel1Weights, 36 * 16 * 4 * sizeof(float));
            memcpy(paramModel1Alpha, fx2a_paramModel1Alpha, 16 * sizeof(float));
            memcpy(paramModel2Bias, fx2a_paramModel2Bias, 16 * sizeof(float));
            memcpy(paramModel2Weights, fx2a_paramModel2Weights, 36 * 16 * 4 * sizeof(float));
            memcpy(paramModel2Alpha, fx2a_paramModel2Alpha, 16 * sizeof(float));
            memcpy(paramModel3Bias, fx2a_paramModel3Bias, 16 * sizeof(float));
            memcpy(paramModel3Weights, fx2a_paramModel3Weights, 36 * 16 * 4 * sizeof(float));
            memcpy(paramModel3Alpha, fx2a_paramModel3Alpha, 16 * sizeof(float));
            memcpy(paramModel4Bias, fx2a_paramModel4Bias, 16 * sizeof(float));
            memcpy(paramModel4Weights, fx2a_paramModel4Weights, 36 * 16 * 4 * sizeof(float));
            memcpy(paramModel4Alpha, fx2a_paramModel4Alpha, 16 * sizeof(float));
            memcpy(paramResidualBias, fx2a_paramResidualBias, 16 * sizeof(float));
            memcpy(paramResidualWeights, fx2a_paramResidualWeights, 16 * 4 * 4 * sizeof(float));
            memcpy(paramResidualAlpha, fx2a_paramResidualAlpha, 16 * sizeof(float));
            memcpy(paramSubConvolutionBias, fx2a_paramSubConvolutionBias, 4*scaling * sizeof(float));
            memcpy(paramSubConvolutionWeights, fx2a_paramSubConvolutionWeights, 36 * 16 * 1 * sizeof(float));
            transposeWeights(paramSubConvolutionWeights, 36 * 16 * 1);
            break;
        case 1:
            memcpy(paramFeatureBias, fx2d_paramFeatureBias, 16 * sizeof(float));
            memcpy(paramFeatureWeights, fx2d_paramFeatureWeights, 25 * 4 * 4 * sizeof(float));
            memcpy(paramModel1Bias, fx2d_paramModel1Bias, 16 * sizeof(float));
            memcpy(paramModel1Weights, fx2d_paramModel1Weights, 36 * 16 * 4 * sizeof(float));
            memcpy(paramModel1Alpha, fx2d_paramModel1Alpha, 16 * sizeof(float));
            memcpy(paramModel2Bias, fx2d_paramModel2Bias, 16 * sizeof(float));
            memcpy(paramModel2Weights, fx2d_paramModel2Weights, 36 * 16 * 4 * sizeof(float));
            memcpy(paramModel2Alpha, fx2d_paramModel2Alpha, 16 * sizeof(float));
            memcpy(paramModel3Bias, fx2d_paramModel3Bias, 16 * sizeof(float));
            memcpy(paramModel3Weights, fx2d_paramModel3Weights, 36 * 16 * 4 * sizeof(float));
            memcpy(paramModel3Alpha, fx2d_paramModel3Alpha, 16 * sizeof(float));
            memcpy(paramModel4Bias, fx2d_paramModel4Bias, 16 * sizeof(float));
            memcpy(paramModel4Weights, fx2d_paramModel4Weights, 36 * 16 * 4 * sizeof(float));
            memcpy(paramModel4Alpha, fx2d_paramModel4Alpha, 16 * sizeof(float));
            memcpy(paramResidualBias, fx2d_paramResidualBias, 16 * sizeof(float));
            memcpy(paramResidualWeights, fx2d_paramResidualWeights, 16 * 4 * 4 * sizeof(float));
            memcpy(paramResidualAlpha, fx2d_paramResidualAlpha, 16 * sizeof(float));
            memcpy(paramSubConvolutionBias, fx2d_paramSubConvolutionBias, 4*scaling * sizeof(float));
            memcpy(paramSubConvolutionWeights, fx2d_paramSubConvolutionWeights, 36 * 16 * 1 * sizeof(float));
            transposeWeights(paramSubConvolutionWeights, 36 * 16 * 1);
            break;
        case 2:
            memcpy(paramFeatureBias, fx2_paramFeatureBias, 16 * sizeof(float));
            memcpy(paramFeatureWeights, fx2_paramFeatureWeights, 25 * 4 * 4 * sizeof(float));
            memcpy(paramModel1Bias, fx2_paramModel1Bias, 16 * sizeof(float));
            memcpy(paramModel1Weights, fx2_paramModel1Weights, 36 * 16 * 4 * sizeof(float));
            memcpy(paramModel1Alpha, fx2_paramModel1Alpha, 16 * sizeof(float));
            memcpy(paramModel2Bias, fx2_paramModel2Bias, 16 * sizeof(float));
            memcpy(paramModel2Weights, fx2_paramModel2Weights, 36 * 16 * 4 * sizeof(float));
            memcpy(paramModel2Alpha, fx2_paramModel2Alpha, 16 * sizeof(float));
            memcpy(paramModel3Bias, fx2_paramModel3Bias, 16 * sizeof(float));
            memcpy(paramModel3Weights, fx2_paramModel3Weights, 36 * 16 * 4 * sizeof(float));
            memcpy(paramModel3Alpha, fx2_paramModel3Alpha, 16 * sizeof(float));
            memcpy(paramModel4Bias, fx2_paramModel4Bias, 16 * sizeof(float));
            memcpy(paramModel4Weights, fx2_paramModel4Weights, 36 * 16 * 4 * sizeof(float));
            memcpy(paramModel4Alpha, fx2_paramModel4Alpha, 16 * sizeof(float));
            memcpy(paramResidualBias, fx2_paramResidualBias, 16 * sizeof(float));
            memcpy(paramResidualWeights, fx2_paramResidualWeights, 16 * 4 * 4 * sizeof(float));
            memcpy(paramResidualAlpha, fx2_paramResidualAlpha, 16 * sizeof(float));
            memcpy(paramSubConvolutionBias, fx2_paramSubConvolutionBias, 4*1 * sizeof(float));
            memcpy(paramSubConvolutionWeights, fx2_paramSubConvolutionWeights, 36 * 16 * 1 * sizeof(float));
            transposeWeights(paramSubConvolutionWeights, 36 * 16 * 1);
            break;
        case 3:
            memcpy(paramFeatureBias, fx4_paramFeatureBias, 16 * sizeof(float));
            memcpy(paramFeatureWeights, fx4_paramFeatureWeights, 25 * 4 * 4 * sizeof(float));
            memcpy(paramModel1Bias, fx4_paramModel1Bias, 16 * sizeof(float));
            memcpy(paramModel1Weights, fx4_paramModel1Weights, 36 * 16 * 4 * sizeof(float));
            memcpy(paramModel1Alpha, fx4_paramModel1Alpha, 16 * sizeof(float));
            memcpy(paramModel2Bias, fx4_paramModel2Bias, 16 * sizeof(float));
            memcpy(paramModel2Weights, fx4_paramModel2Weights, 36 * 16 * 4 * sizeof(float));
            memcpy(paramModel2Alpha, fx4_paramModel2Alpha, 16 * sizeof(float));
            memcpy(paramModel3Bias, fx4_paramModel3Bias, 16 * sizeof(float));
            memcpy(paramModel3Weights, fx4_paramModel3Weights, 36 * 16 * 4 * sizeof(float));
            memcpy(paramModel3Alpha, fx4_paramModel3Alpha, 16 * sizeof(float));
            memcpy(paramModel4Bias, fx4_paramModel4Bias, 16 * sizeof(float));
            memcpy(paramModel4Weights, fx4_paramModel4Weights, 36 * 16 * 4 * sizeof(float));
            memcpy(paramModel4Alpha, fx4_paramModel4Alpha, 16 * sizeof(float));
            memcpy(paramResidualBias, fx4_paramResidualBias, 16 * sizeof(float));
            memcpy(paramResidualWeights, fx4_paramResidualWeights, 16 * 4 * 4 * sizeof(float));
            memcpy(paramResidualAlpha, fx4_paramResidualAlpha, 16 * sizeof(float));
            memcpy(paramSubConvolutionBias, fx42_paramSubConvolutionBias, 4*1 * sizeof(float));
            memcpy(paramSubConvolutionWeights, fx42_paramSubConvolutionWeights, 36 * 16 * 1 * sizeof(float));
            transposeWeights(paramSubConvolutionWeights, 36 * 16 * 1);
            break;
        case 4:
            memcpy(paramFeatureBias, fx3_paramFeatureBias, 16 * sizeof(float));
            memcpy(paramFeatureWeights, fx3_paramFeatureWeights, 25 * 4 * 4 * sizeof(float));
            memcpy(paramModel1Bias, fx3_paramModel1Bias, 16 * sizeof(float));
            memcpy(paramModel1Weights, fx3_paramModel1Weights, 36 * 16 * 4 * sizeof(float));
            memcpy(paramModel1Alpha, fx3_paramModel1Alpha, 16 * sizeof(float));
            memcpy(paramModel2Bias, fx3_paramModel2Bias, 16 * sizeof(float));
            memcpy(paramModel2Weights, fx3_paramModel2Weights, 36 * 16 * 4 * sizeof(float));
            memcpy(paramModel2Alpha, fx3_paramModel2Alpha, 16 * sizeof(float));
            memcpy(paramModel3Bias, fx3_paramModel3Bias, 16 * sizeof(float));
            memcpy(paramModel3Weights, fx3_paramModel3Weights, 36 * 16 * 4 * sizeof(float));
            memcpy(paramModel3Alpha, fx3_paramModel3Alpha, 16 * sizeof(float));
            memcpy(paramModel4Bias, fx3_paramModel4Bias, 16 * sizeof(float));
            memcpy(paramModel4Weights, fx3_paramModel4Weights, 36 * 16 * 4 * sizeof(float));
            memcpy(paramModel4Alpha, fx3_paramModel4Alpha, 16 * sizeof(float));
            memcpy(paramResidualBias, fx3_paramResidualBias, 16 * sizeof(float));
            memcpy(paramResidualWeights, fx3_paramResidualWeights, 16 * 4 * 4 * sizeof(float));
            memcpy(paramResidualAlpha, fx3_paramResidualAlpha, 16 * sizeof(float));
            memcpy(paramSubConvolutionBias, fx3_paramSubConvolutionBias, 4*scaling * sizeof(float));
            memcpy(paramSubConvolutionWeights, fx3_paramSubConvolutionWeights, 36 * 16 * scaling * sizeof(float));
            transposeWeights(paramSubConvolutionWeights, 36 * 16 * scaling);
            break;
        case 5:
            memcpy(paramFeatureBias, fx4_paramFeatureBias, 16 * sizeof(float));
            memcpy(paramFeatureWeights, fx4_paramFeatureWeights, 25 * 4 * 4 * sizeof(float));
            memcpy(paramModel1Bias, fx4_paramModel1Bias, 16 * sizeof(float));
            memcpy(paramModel1Weights, fx4_paramModel1Weights, 36 * 16 * 4 * sizeof(float));
            memcpy(paramModel1Alpha, fx4_paramModel1Alpha, 16 * sizeof(float));
            memcpy(paramModel2Bias, fx4_paramModel2Bias, 16 * sizeof(float));
            memcpy(paramModel2Weights, fx4_paramModel2Weights, 36 * 16 * 4 * sizeof(float));
            memcpy(paramModel2Alpha, fx4_paramModel2Alpha, 16 * sizeof(float));
            memcpy(paramModel3Bias, fx4_paramModel3Bias, 16 * sizeof(float));
            memcpy(paramModel3Weights, fx4_paramModel3Weights, 36 * 16 * 4 * sizeof(float));
            memcpy(paramModel3Alpha, fx4_paramModel3Alpha, 16 * sizeof(float));
            memcpy(paramModel4Bias, fx4_paramModel4Bias, 16 * sizeof(float));
            memcpy(paramModel4Weights, fx4_paramModel4Weights, 36 * 16 * 4 * sizeof(float));
            memcpy(paramModel4Alpha, fx4_paramModel4Alpha, 16 * sizeof(float));
            memcpy(paramResidualBias, fx4_paramResidualBias, 16 * sizeof(float));
            memcpy(paramResidualWeights, fx4_paramResidualWeights, 16 * 4 * 4 * sizeof(float));
            memcpy(paramResidualAlpha, fx4_paramResidualAlpha, 16 * sizeof(float));
            memcpy(paramSubConvolutionBias, fx4_paramSubConvolutionBias, 4*scaling * sizeof(float));
            memcpy(paramSubConvolutionWeights, fx4_paramSubConvolutionWeights, 36 * 16 * scaling * sizeof(float));
            transposeWeights(paramSubConvolutionWeights, 36 * 16 * scaling);
            break;
        default:
            ADM_assert(0);
            break;
    }    
    transposeWeights(paramModel1Weights, 36 * 16 * 4);
    transposeWeights(paramModel2Weights, 36 * 16 * 4);
    transposeWeights(paramModel3Weights, 36 * 16 * 4);
    transposeWeights(paramModel4Weights, 36 * 16 * 4);
    transposeWeights(paramResidualWeights, 16 * 4 * 4);
}

FSRCNN::~FSRCNN()
{
    delete paddedImg;
    delete [] worker_threads;
    delete [] worker_thread_args;
    ADM_dezalloc(layerFeature);
    ADM_dezalloc(layerModel1);
    ADM_dezalloc(layerModel2);
    ADM_dezalloc(layerModel3);
    ADM_dezalloc(layerModel4);
    ADM_dezalloc(layerResidual);
    ADM_dezalloc(paramFeatureBias);
    ADM_dezalloc(paramFeatureWeights);
    ADM_dezalloc(paramModel1Bias);
    ADM_dezalloc(paramModel1Weights);
    ADM_dezalloc(paramModel1Alpha);
    ADM_dezalloc(paramModel2Bias);
    ADM_dezalloc(paramModel2Weights);
    ADM_dezalloc(paramModel2Alpha);
    ADM_dezalloc(paramModel3Bias);
    ADM_dezalloc(paramModel3Weights);
    ADM_dezalloc(paramModel3Alpha);
    ADM_dezalloc(paramModel4Bias);
    ADM_dezalloc(paramModel4Weights);
    ADM_dezalloc(paramModel4Alpha);
    ADM_dezalloc(paramResidualBias);
    ADM_dezalloc(paramResidualWeights);
    ADM_dezalloc(paramResidualAlpha);
    ADM_dezalloc(paramSubConvolutionBias);
    ADM_dezalloc(paramSubConvolutionWeights);
}

void FSRCNN::upscaleY(ADMImage *srcImg, ADMImage *dstImg)
{
    ADM_assert(srcImg->_width == w);
    ADM_assert(srcImg->_height == h);
    ADM_assert(dstImg->_width == w*scaling);
    ADM_assert(dstImg->_height == h*scaling);

    // create padded Y plane
    uint8_t * srcImgPlane = srcImg->GetReadPtr(PLANAR_Y);
    unsigned int srcImgStride = srcImg->GetPitch(PLANAR_Y);
    uint8_t * pp = paddedImgPlane;
    pp += 2*paddedImgStride;
    for (int y=0; y<h; y++)
    {
        memcpy(pp+2, srcImgPlane, w);
        pp += paddedImgStride;
        srcImgPlane += srcImgStride;
    }
    pp = paddedImgPlane;
    pp += 2*paddedImgStride;    
    for (int y=0; y<h; y++)
    {
        pp[0] = pp[1] = pp[2];
        pp[w+3] = pp[w+2] = pp[w+1];
        pp += paddedImgStride;    
    }
    memcpy(paddedImgPlane, paddedImgPlane + 2*paddedImgStride, w+4);
    memcpy(paddedImgPlane + paddedImgStride, paddedImgPlane + 2*paddedImgStride, w+4);
    memcpy(paddedImgPlane + (h+2)*paddedImgStride, paddedImgPlane + (h+1)*paddedImgStride, w+4);
    memcpy(paddedImgPlane + (h+3)*paddedImgStride, paddedImgPlane + (h+1)*paddedImgStride, w+4);
    // got padded Y plane
    
    // Feature layer
    for (int tr=0; tr<threads; tr++)
    {
        worker_thread_args[tr].w = w;
        worker_thread_args[tr].h = h;
        worker_thread_args[tr].ystart = tr;
        worker_thread_args[tr].yincr = threads;
        worker_thread_args[tr].scaling = scaling;
        worker_thread_args[tr].imgPlane = paddedImgPlane;
        worker_thread_args[tr].imgStride = paddedImgStride;
        worker_thread_args[tr].featureLayer = NULL;
        worker_thread_args[tr].inputLayer = NULL;
        worker_thread_args[tr].bias = paramFeatureBias;
        worker_thread_args[tr].weights = paramFeatureWeights;
        worker_thread_args[tr].alpha = NULL;
        worker_thread_args[tr].outputLayer = layerFeature;
    }

    for (int tr=0; tr<threads; tr++)
    {
        pthread_create( &worker_threads[tr], NULL, feature_worker_thread, (void*) &worker_thread_args[tr]);
    }
    // work in thread workers...
    for (int tr=0; tr<threads; tr++)
    {
        pthread_join( worker_threads[tr], NULL);
    }
    
    // Model1 layer
    for (int tr=0; tr<threads; tr++)
    {
        worker_thread_args[tr].w = w;
        worker_thread_args[tr].h = h;
        worker_thread_args[tr].ystart = tr;
        worker_thread_args[tr].yincr = threads;
        worker_thread_args[tr].scaling = scaling;
        worker_thread_args[tr].imgPlane = NULL;
        worker_thread_args[tr].imgStride = 0;
        worker_thread_args[tr].featureLayer = NULL;
        worker_thread_args[tr].inputLayer = layerFeature;
        worker_thread_args[tr].bias = paramModel1Bias;
        worker_thread_args[tr].weights = paramModel1Weights;
        worker_thread_args[tr].alpha = paramModel1Alpha;
        worker_thread_args[tr].outputLayer = layerModel1;
    }

    for (int tr=0; tr<threads; tr++)
    {
        pthread_create( &worker_threads[tr], NULL, model_worker_thread, (void*) &worker_thread_args[tr]);
    }
    // work in thread workers...
    for (int tr=0; tr<threads; tr++)
    {
        pthread_join( worker_threads[tr], NULL);
    }

    // Model2 layer
    for (int tr=0; tr<threads; tr++)
    {
        worker_thread_args[tr].w = w;
        worker_thread_args[tr].h = h;
        worker_thread_args[tr].ystart = tr;
        worker_thread_args[tr].yincr = threads;
        worker_thread_args[tr].scaling = scaling;
        worker_thread_args[tr].imgPlane = NULL;
        worker_thread_args[tr].imgStride = 0;
        worker_thread_args[tr].featureLayer = NULL;
        worker_thread_args[tr].inputLayer = layerModel1;
        worker_thread_args[tr].bias = paramModel2Bias;
        worker_thread_args[tr].weights = paramModel2Weights;
        worker_thread_args[tr].alpha = paramModel2Alpha;
        worker_thread_args[tr].outputLayer = layerModel2;
    }

    for (int tr=0; tr<threads; tr++)
    {
        pthread_create( &worker_threads[tr], NULL, model_worker_thread, (void*) &worker_thread_args[tr]);
    }
    // work in thread workers...
    for (int tr=0; tr<threads; tr++)
    {
        pthread_join( worker_threads[tr], NULL);
    }
    
    // Model3 layer
    for (int tr=0; tr<threads; tr++)
    {
        worker_thread_args[tr].w = w;
        worker_thread_args[tr].h = h;
        worker_thread_args[tr].ystart = tr;
        worker_thread_args[tr].yincr = threads;
        worker_thread_args[tr].scaling = scaling;
        worker_thread_args[tr].imgPlane = NULL;
        worker_thread_args[tr].imgStride = 0;
        worker_thread_args[tr].featureLayer = NULL;
        worker_thread_args[tr].inputLayer = layerModel2;
        worker_thread_args[tr].bias = paramModel3Bias;
        worker_thread_args[tr].weights = paramModel3Weights;
        worker_thread_args[tr].alpha = paramModel3Alpha;
        worker_thread_args[tr].outputLayer = layerModel3;
    }

    for (int tr=0; tr<threads; tr++)
    {
        pthread_create( &worker_threads[tr], NULL, model_worker_thread, (void*) &worker_thread_args[tr]);
    }
    // work in thread workers...
    for (int tr=0; tr<threads; tr++)
    {
        pthread_join( worker_threads[tr], NULL);
    }
    
    // Model4 layer
    for (int tr=0; tr<threads; tr++)
    {
        worker_thread_args[tr].w = w;
        worker_thread_args[tr].h = h;
        worker_thread_args[tr].ystart = tr;
        worker_thread_args[tr].yincr = threads;
        worker_thread_args[tr].scaling = scaling;
        worker_thread_args[tr].imgPlane = NULL;
        worker_thread_args[tr].imgStride = 0;
        worker_thread_args[tr].featureLayer = NULL;
        worker_thread_args[tr].inputLayer = layerModel3;
        worker_thread_args[tr].bias = paramModel4Bias;
        worker_thread_args[tr].weights = paramModel4Weights;
        worker_thread_args[tr].alpha = paramModel4Alpha;
        worker_thread_args[tr].outputLayer = layerModel4;
    }

    for (int tr=0; tr<threads; tr++)
    {
        pthread_create( &worker_threads[tr], NULL, model_worker_thread, (void*) &worker_thread_args[tr]);
    }
    // work in thread workers...
    for (int tr=0; tr<threads; tr++)
    {
        pthread_join( worker_threads[tr], NULL);
    }
    
    // Residual layer
    for (int tr=0; tr<threads; tr++)
    {
        worker_thread_args[tr].w = w;
        worker_thread_args[tr].h = h;
        worker_thread_args[tr].ystart = tr;
        worker_thread_args[tr].yincr = threads;
        worker_thread_args[tr].scaling = scaling;
        worker_thread_args[tr].imgPlane = NULL;
        worker_thread_args[tr].imgStride = 0;
        worker_thread_args[tr].featureLayer = layerFeature;
        worker_thread_args[tr].inputLayer = layerModel4;
        worker_thread_args[tr].bias = paramResidualBias;
        worker_thread_args[tr].weights = paramResidualWeights;
        worker_thread_args[tr].alpha = paramResidualAlpha;
        worker_thread_args[tr].outputLayer = layerResidual;
    }

    for (int tr=0; tr<threads; tr++)
    {
        pthread_create( &worker_threads[tr], NULL, residual_worker_thread, (void*) &worker_thread_args[tr]);
    }
    // work in thread workers...
    for (int tr=0; tr<threads; tr++)
    {
        pthread_join( worker_threads[tr], NULL);
    }

    // Subconvolutional layer
    uint8_t * dstImgPlane = dstImg->GetReadPtr(PLANAR_Y);
    unsigned int dstImgStride = dstImg->GetPitch(PLANAR_Y);
    for (int tr=0; tr<threads; tr++)
    {
        worker_thread_args[tr].w = w;
        worker_thread_args[tr].h = h;
        worker_thread_args[tr].ystart = tr;
        worker_thread_args[tr].yincr = threads;
        worker_thread_args[tr].scaling = scaling;
        worker_thread_args[tr].imgPlane = dstImgPlane;
        worker_thread_args[tr].imgStride = dstImgStride;
        worker_thread_args[tr].featureLayer = NULL;
        worker_thread_args[tr].inputLayer = layerResidual;
        worker_thread_args[tr].bias = paramSubConvolutionBias;
        worker_thread_args[tr].weights = paramSubConvolutionWeights;
        worker_thread_args[tr].alpha = NULL;
        worker_thread_args[tr].outputLayer = NULL;
    }

    for (int tr=0; tr<threads; tr++)
    {
        switch (scaling)
        {
            case 2:
                pthread_create( &worker_threads[tr], NULL, subconv2_worker_thread, (void*) &worker_thread_args[tr]);
                break;
            case 3:
                pthread_create( &worker_threads[tr], NULL, subconv3_worker_thread, (void*) &worker_thread_args[tr]);
                break;
            case 4:
                pthread_create( &worker_threads[tr], NULL, subconv4_worker_thread, (void*) &worker_thread_args[tr]);
                break;
            default:
                ADM_assert(0);
                break;
        }
    }
    // work in thread workers...
    for (int tr=0; tr<threads; tr++)
    {
        pthread_join( worker_threads[tr], NULL);
    }

    dstImg->copyInfo(srcImg);
}

void * FSRCNN::feature_worker_thread( void *ptr )
{
    worker_thread_arg * arg = (worker_thread_arg*)ptr;
    int w = arg->w;
    int h = arg->h;
    int ystart = arg->ystart;
    int yincr = arg->yincr;
    
#ifdef USE_SSE2    
    __m128 t[4];
#else    
    float t[16];
#endif
    float pix;
    
    for (int y=ystart; y<h; y+=yincr)
    {
        for (int x=0; x<w; x++)
        {    
            m_load4_bias(t, arg->bias);
            int ws = 0;
            for (int p=-2; p<=2; p++)
            {
                for (int q=-2; q<=2; q++)
                {
                    pix = arg->imgPlane[x + 2 + p + (y + 2 + q)*arg->imgStride] / 255.0;
                    m_add4_vecXs(t, arg->weights + ws, pix);
                    ws += 16; 
                }
            }
            m_store4(t, arg->outputLayer + ((y+1)*(w+2) + (x+1))*4*4);
        }
    }
    
    pthread_exit(NULL);
    return NULL;    
}

void * FSRCNN::model_worker_thread( void *ptr )
{
    worker_thread_arg * arg = (worker_thread_arg*)ptr;
    int w = arg->w;
    int h = arg->h;
    int ystart = arg->ystart;
    int yincr = arg->yincr;
    
#ifdef USE_SSE2    
    __m128 t[4];
#else    
    float t[16];
#endif
    
    for (int y=ystart; y<h; y+=yincr)
    {
        for (int x=0; x<w; x++)
        {    
            m_load4_bias(t, arg->bias);
            int ws = 0;
            for (int p=-1; p<=1; p++)
            {
                for (int q=-1; q<=1; q++)
                {
                    m_add4_mxXvec4(t, arg->weights + ws, arg->inputLayer + ((y+1+q)*(w+2) + (x+1+p))*4*4);
                    ws += 256;
                }
            }
            m_alpha4(t, arg->alpha);
            m_store4(t, arg->outputLayer + ((y+1)*(w+2) + (x+1))*4*4);
        }
    }
    
    pthread_exit(NULL);
    return NULL;    
}

void * FSRCNN::residual_worker_thread( void *ptr )
{
    worker_thread_arg * arg = (worker_thread_arg*)ptr;
    int w = arg->w;
    int h = arg->h;
    int ystart = arg->ystart;
    int yincr = arg->yincr;
    
#ifdef USE_SSE2    
    __m128 t[4];
#else    
    float t[16];
#endif
    
    for (int y=ystart; y<h; y+=yincr)
    {
        for (int x=0; x<w; x++)
        {    
            m_load4_bias(t, arg->bias);
            m_add4_mxXvec4(t, arg->weights, arg->inputLayer + ((y+1)*(w+2) + (x+1))*4*4);
            m_add4_vec(t, arg->featureLayer + ((y+1)*(w+2) + (x+1))*4*4);
            m_alpha4(t, arg->alpha);
            m_store4(t, arg->outputLayer + ((y+1)*(w+2) + (x+1))*4*4);
        }
    }
    
    pthread_exit(NULL);
    return NULL;    
}

void * FSRCNN::subconv2_worker_thread( void *ptr )
{
    worker_thread_arg * arg = (worker_thread_arg*)ptr;
    int w = arg->w;
    int h = arg->h;
    int ystart = arg->ystart;
    int yincr = arg->yincr;
    
#ifdef USE_SSE2    
    __m128 t[4];
#else    
    float t[16];
#endif
    
    for (int y=ystart; y<h; y+=yincr)
    {
        for (int x=0; x<w; x++)
        {    
            m_load1_bias(t, arg->bias);
            int ws = 0;
            for (int p=-1; p<=1; p++)
            {
                for (int q=-1; q<=1; q++)
                {

                    m_add1_mxXvec4(t, arg->weights + ws, arg->inputLayer + ((y+1+q)*(w+2) + (x+1+p))*4*4);
                    ws += 64;
                }
            }
            m_integerize1(t, arg->imgPlane + ((y*2)*arg->imgStride + (x*2)), arg->imgStride);
        }
    }
    
    pthread_exit(NULL);
    return NULL;    
}

void * FSRCNN::subconv3_worker_thread( void *ptr )
{
    worker_thread_arg * arg = (worker_thread_arg*)ptr;
    int w = arg->w;
    int h = arg->h;
    int ystart = arg->ystart;
    int yincr = arg->yincr;
    
#ifdef USE_SSE2    
    __m128 t[4];
#else    
    float t[16];
#endif
    
    for (int y=ystart; y<h; y+=yincr)
    {
        for (int x=0; x<w; x++)
        {    
            m_load4_bias(t, arg->bias);
            int ws = 0;
            for (int p=-1; p<=1; p++)
            {
                for (int q=-1; q<=1; q++)
                {

                    m_add3_mxXvec4(t, arg->weights + ws, arg->inputLayer + ((y+1+q)*(w+2) + (x+1+p))*4*4);
                    ws += 192;
                }
            }
            m_integerize3(t, arg->imgPlane + ((y*3)*arg->imgStride + (x*3)), arg->imgStride);
        }
    }
    
    pthread_exit(NULL);
    return NULL;    
}

void * FSRCNN::subconv4_worker_thread( void *ptr )
{
    worker_thread_arg * arg = (worker_thread_arg*)ptr;
    int w = arg->w;
    int h = arg->h;
    int ystart = arg->ystart;
    int yincr = arg->yincr;
    
#ifdef USE_SSE2    
    __m128 t[4];
#else    
    float t[16];
#endif
    
    for (int y=ystart; y<h; y+=yincr)
    {
        for (int x=0; x<w; x++)
        {    
            m_load4_bias(t, arg->bias);
            int ws = 0;
            for (int p=-1; p<=1; p++)
            {
                for (int q=-1; q<=1; q++)
                {

                    m_add4_mxXvec4(t, arg->weights + ws, arg->inputLayer + ((y+1+q)*(w+2) + (x+1+p))*4*4);
                    ws += 256;
                }
            }
            m_integerize4(t, arg->imgPlane + ((y*4)*arg->imgStride + (x*4)), arg->imgStride);
        }
    }
    
    pthread_exit(NULL);
    return NULL;    
}

void FSRCNN::transposeWeights(float * weights, int weightCount)
{
#ifdef USE_SSE2
    return;
#else
    float tmp[16];
    for (int k=0; k<(weightCount/16); k++)
    {
        for (int i=0; i<4; i++)
        {
            for (int j=0; j<4; j++)
            {
                tmp[j*4+i] = weights[i*4+j];
            }
        }
        for (int i=0; i<16; i++)
        {
            weights[i] = tmp[i];
        }
        weights += 16;
    }
#endif
}

// Vectorizable functions

#ifdef USE_SSE2
void FSRCNN::m_load1_bias(__m128 * t, float * bias)
{
    t[0] = _mm_load_ps(bias);
}
#else    
void FSRCNN::m_load1_bias(float * t, float * bias)
{
    memcpy(t, bias, 4*1*sizeof(float));
}
#endif

#ifdef USE_SSE2    
void FSRCNN::m_load3_bias(__m128 * t, float * bias)
{
    t[0] = _mm_load_ps(bias + 0);
    t[1] = _mm_load_ps(bias + 4);
    t[2] = _mm_load_ps(bias + 8);
}
#else    
void FSRCNN::m_load3_bias(float * t, float * bias)
{
    memcpy(t, bias, 4*3*sizeof(float));
}
#endif 

#ifdef USE_SSE2    
void FSRCNN::m_load4_bias(__m128 * t, float * bias)
{
    t[0] = _mm_load_ps(bias + 0);
    t[1] = _mm_load_ps(bias + 4);
    t[2] = _mm_load_ps(bias + 8);
    t[3] = _mm_load_ps(bias + 12);
}
#else    
void FSRCNN::m_load4_bias(float * t, float * bias)
{
    memcpy(t, bias, 4*4*sizeof(float));
}
#endif 

#ifdef USE_SSE2   
void FSRCNN::m_add4_vec(__m128 * t, float * vec)
{
    __m128 v1 = _mm_load_ps(vec + 0);
    __m128 v2 = _mm_load_ps(vec + 4);
    __m128 v3 = _mm_load_ps(vec + 8);
    __m128 v4 = _mm_load_ps(vec + 12);
    t[0] = _mm_add_ps(t[0], v1);
    t[1] = _mm_add_ps(t[1], v2);
    t[2] = _mm_add_ps(t[2], v3);
    t[3] = _mm_add_ps(t[3], v4);
}
#else    
void FSRCNN::m_add4_vec(float * t, float * vec)
{
    for (int i=0; i<4*4; i++)
    {
        t[i] += vec[i];
    }
}
#endif 

#ifdef USE_SSE2
void FSRCNN::m_add4_vecXs(__m128 * t, float * vec, float scalar)
{
    __m128 s = _mm_load1_ps(&scalar);
    __m128 v1 = _mm_load_ps(vec + 0);
    __m128 v2 = _mm_load_ps(vec + 4);
    __m128 v3 = _mm_load_ps(vec + 8);
    __m128 v4 = _mm_load_ps(vec + 12);
    t[0] = _mm_add_ps(t[0], _mm_mul_ps(v1, s));
    t[1] = _mm_add_ps(t[1], _mm_mul_ps(v2, s));
    t[2] = _mm_add_ps(t[2], _mm_mul_ps(v3, s));
    t[3] = _mm_add_ps(t[3], _mm_mul_ps(v4, s));    
}
#else    
void FSRCNN::m_add4_vecXs(float * t, float * vec, float scalar)
{
    for (int i=0; i<4*4; i++)
    {
        t[i] += (vec[i] * scalar);
    }
}
#endif 

#ifdef USE_SSE2
void FSRCNN::m_store1(__m128 * t, float * layer)
{
    _mm_store_ps(layer, t[0]);
}
#else    
void FSRCNN::m_store1(float * t, float * layer)
{
    memcpy(layer, t, 4*1*sizeof(float));
}
#endif 

#ifdef USE_SSE2
void FSRCNN::m_store3(__m128 * t, float * layer)
{
    _mm_store_ps(layer + 0, t[0]);
    _mm_store_ps(layer + 4, t[1]);
    _mm_store_ps(layer + 8, t[2]);
}
#else    
void FSRCNN::m_store3(float * t, float * layer)
{
    memcpy(layer, t, 4*3*sizeof(float));
}
#endif 

#ifdef USE_SSE2
void FSRCNN::m_store4(__m128 * t, float * layer)
{
    _mm_store_ps(layer + 0, t[0]);
    _mm_store_ps(layer + 4, t[1]);
    _mm_store_ps(layer + 8, t[2]);
    _mm_store_ps(layer + 12, t[3]);
}
#else    
void FSRCNN::m_store4(float * t, float * layer)
{
    memcpy(layer, t, 4*4*sizeof(float));
}
#endif 

#ifdef USE_SSE2
void FSRCNN::m_add1_mxXvec4(__m128 * t, float * mx, float * vec)
{
    for (int i=0; i<4; i++)
    {
        __m128 v4 = _mm_load_ps(vec);
        for (int m=0; m<1; m++)
        {
            __m128 u1 = _mm_shuffle_ps(v4,v4, _MM_SHUFFLE(0,0,0,0));
            __m128 u2 = _mm_shuffle_ps(v4,v4, _MM_SHUFFLE(1,1,1,1));
            __m128 u3 = _mm_shuffle_ps(v4,v4, _MM_SHUFFLE(2,2,2,2));
            __m128 u4 = _mm_shuffle_ps(v4,v4, _MM_SHUFFLE(3,3,3,3));
            __m128 prod1 = _mm_mul_ps(u1, _mm_load_ps(mx));
            mx+=4;
            __m128 prod2 = _mm_mul_ps(u2, _mm_load_ps(mx));
            mx+=4;
            __m128 prod3 = _mm_mul_ps(u3, _mm_load_ps(mx));
            mx+=4;
            __m128 prod4 = _mm_mul_ps(u4, _mm_load_ps(mx));            
            mx+=4;
            t[m] = _mm_add_ps(t[m],_mm_add_ps(_mm_add_ps(prod1, prod2), _mm_add_ps(prod3, prod4)));
        }
        vec += 4;
    }      
}
#else    
void FSRCNN::m_add1_mxXvec4(float * t, float * mx, float * vec)
{
    for (int i=0; i<4; i++)
    {
        for (int m=0; m<1; m++)
        {
            for (int j=0; j<4; j++)
            {
                float sum = 0;
                for (int k=0; k<4; k++)
                {
                    sum += *mx * vec[k];
                    mx++;
                }
                t[m*4+j] += sum;
            }
        }
        vec += 4;
    }
}
#endif 

#ifdef USE_SSE2
void FSRCNN::m_add3_mxXvec4(__m128 * t, float * mx, float * vec)
{
    for (int i=0; i<4; i++)
    {
        __m128 v4 = _mm_load_ps(vec);
        for (int m=0; m<3; m++)
        {
            __m128 u1 = _mm_shuffle_ps(v4,v4, _MM_SHUFFLE(0,0,0,0));
            __m128 u2 = _mm_shuffle_ps(v4,v4, _MM_SHUFFLE(1,1,1,1));
            __m128 u3 = _mm_shuffle_ps(v4,v4, _MM_SHUFFLE(2,2,2,2));
            __m128 u4 = _mm_shuffle_ps(v4,v4, _MM_SHUFFLE(3,3,3,3));
            __m128 prod1 = _mm_mul_ps(u1, _mm_load_ps(mx));
            mx+=4;
            __m128 prod2 = _mm_mul_ps(u2, _mm_load_ps(mx));
            mx+=4;
            __m128 prod3 = _mm_mul_ps(u3, _mm_load_ps(mx));
            mx+=4;
            __m128 prod4 = _mm_mul_ps(u4, _mm_load_ps(mx));            
            mx+=4;
            t[m] = _mm_add_ps(t[m],_mm_add_ps(_mm_add_ps(prod1, prod2), _mm_add_ps(prod3, prod4)));
        }
        vec += 4;
    }      
}
#else    
void FSRCNN::m_add3_mxXvec4(float * t, float * mx, float * vec)
{
    for (int i=0; i<4; i++)
    {
        for (int m=0; m<3; m++)
        {
            for (int j=0; j<4; j++)
            {
                float sum = 0;
                for (int k=0; k<4; k++)
                {
                    sum += *mx * vec[k];
                    mx++;
                }
                t[m*4+j] += sum;
            }
        }
        vec += 4;
    }
}
#endif 

#ifdef USE_SSE2
void FSRCNN::m_add4_mxXvec4(__m128 * t, float * mx, float * vec)
{
    for (int i=0; i<4; i++)
    {
        __m128 v4 = _mm_load_ps(vec);
        for (int m=0; m<4; m++)
        {
            __m128 u1 = _mm_shuffle_ps(v4,v4, _MM_SHUFFLE(0,0,0,0));
            __m128 u2 = _mm_shuffle_ps(v4,v4, _MM_SHUFFLE(1,1,1,1));
            __m128 u3 = _mm_shuffle_ps(v4,v4, _MM_SHUFFLE(2,2,2,2));
            __m128 u4 = _mm_shuffle_ps(v4,v4, _MM_SHUFFLE(3,3,3,3));
            __m128 prod1 = _mm_mul_ps(u1, _mm_load_ps(mx));
            mx+=4;
            __m128 prod2 = _mm_mul_ps(u2, _mm_load_ps(mx));
            mx+=4;
            __m128 prod3 = _mm_mul_ps(u3, _mm_load_ps(mx));
            mx+=4;
            __m128 prod4 = _mm_mul_ps(u4, _mm_load_ps(mx));            
            mx+=4;
            t[m] = _mm_add_ps(t[m],_mm_add_ps(_mm_add_ps(prod1, prod2), _mm_add_ps(prod3, prod4)));
        }
        vec += 4;
    }      
}
#else    
void FSRCNN::m_add4_mxXvec4(float * t, float * mx, float * vec)
{
    for (int i=0; i<4; i++)
    {
        for (int m=0; m<4; m++)
        {
            for (int j=0; j<4; j++)
            {
                float sum = 0;
                for (int k=0; k<4; k++)
                {
                    sum += *mx * vec[k];
                    mx++;
                }
                t[m*4+j] += sum;
            }
        }
        vec += 4;
    }
}
#endif 

#ifdef USE_SSE2
void FSRCNN::m_alpha4(__m128 * t, float * alpha)
{
    __m128 zero = _mm_setzero_ps();
    __m128 a1 = _mm_load_ps(alpha + 0);
    __m128 a2 = _mm_load_ps(alpha + 4);
    __m128 a3 = _mm_load_ps(alpha + 8);
    __m128 a4 = _mm_load_ps(alpha + 12);
    t[0] = _mm_add_ps(_mm_max_ps(t[0], zero), _mm_mul_ps(_mm_min_ps(t[0], zero), a1));
    t[1] = _mm_add_ps(_mm_max_ps(t[1], zero), _mm_mul_ps(_mm_min_ps(t[1], zero), a2));
    t[2] = _mm_add_ps(_mm_max_ps(t[2], zero), _mm_mul_ps(_mm_min_ps(t[2], zero), a3));
    t[3] = _mm_add_ps(_mm_max_ps(t[3], zero), _mm_mul_ps(_mm_min_ps(t[3], zero), a4));
}
#else    
void FSRCNN::m_alpha4(float * t, float * alpha)
{
    for (int i=0; i<4*4; i++)
    {
        t[i] = ((t[i] < 0) ? 0.0 : t[i]) + alpha[i]*((t[i] > 0) ? 0.0 : t[i]);
    }
}
#endif 

#ifdef USE_SSE2
void FSRCNN::m_integerize1(__m128 * t, uint8_t * plane, unsigned int stride)
{
    float c255 = 255;
    __m128 zero = _mm_setzero_ps();
    __m128 v255 = _mm_load1_ps(&c255);
    t[0] = _mm_mul_ps(t[0], v255);
    t[0] = _mm_max_ps(t[0], zero);
    t[0] = _mm_min_ps(t[0], v255);
    __m128i intvalue;
    uint32_t pixels;
    uint8_t * pix = ((uint8_t*)&pixels);
    intvalue = _mm_cvtps_epi32(t[0]);
    intvalue = _mm_packs_epi32(intvalue,intvalue);
    intvalue = _mm_packus_epi16(intvalue,intvalue);
    pixels = _mm_cvtsi128_si32(intvalue);
    plane[0+0] = pix[0];
    plane[0+1] = pix[2];
    plane[stride+0] = pix[1];
    plane[stride+1] = pix[3];
}
#else    
void FSRCNN::m_integerize1(float * t, uint8_t * plane, unsigned int stride)
{
    uint8_t pix[4];
    for (int i=0; i<4*1; i++)
    {
        t[i] *= 255.0;
        t[i] = std::round(t[i]);
        if (t[i] < 0) t[i] = 0;
        if (t[i] > 255) t[i] = 255;
        pix[i] = t[i];
    }
    plane[0+0] = pix[0];
    plane[0+1] = pix[2];
    plane[stride+0] = pix[1];
    plane[stride+1] = pix[3];
}
#endif 

#ifdef USE_SSE2
void FSRCNN::m_integerize3(__m128 * t, uint8_t * plane, unsigned int stride)
{
    float c255 = 255;
    __m128 zero = _mm_setzero_ps();
    __m128 v255 = _mm_load1_ps(&c255);
    t[0] = _mm_mul_ps(t[0], v255);
    t[0] = _mm_max_ps(t[0], zero);
    t[0] = _mm_min_ps(t[0], v255);
    t[1] = _mm_mul_ps(t[1], v255);
    t[1] = _mm_max_ps(t[1], zero);
    t[1] = _mm_min_ps(t[1], v255);
    t[2] = _mm_mul_ps(t[2], v255);
    t[2] = _mm_max_ps(t[2], zero);
    t[2] = _mm_min_ps(t[2], v255);
    __m128i intvalue;
    uint32_t pixels;
    uint8_t * pix = ((uint8_t*)&pixels);
    intvalue = _mm_cvtps_epi32(t[0]);
    intvalue = _mm_packs_epi32(intvalue,intvalue);
    intvalue = _mm_packus_epi16(intvalue,intvalue);
    pixels = _mm_cvtsi128_si32(intvalue);
    plane[0+0] = pix[0];
    plane[stride+0] = pix[1];
    plane[2*stride+0] = pix[2];
    intvalue = _mm_cvtps_epi32(t[1]);
    intvalue = _mm_packs_epi32(intvalue,intvalue);
    intvalue = _mm_packus_epi16(intvalue,intvalue);
    pixels = _mm_cvtsi128_si32(intvalue);
    plane[0+1] = pix[0];
    plane[stride+1] = pix[1];
    plane[2*stride+1] = pix[2];
    intvalue = _mm_cvtps_epi32(t[2]);
    intvalue = _mm_packs_epi32(intvalue,intvalue);
    intvalue = _mm_packus_epi16(intvalue,intvalue);
    pixels = _mm_cvtsi128_si32(intvalue);
    plane[0+2] = pix[0];
    plane[stride+2] = pix[1];
    plane[2*stride+2] = pix[2];
}
#else    
void FSRCNN::m_integerize3(float * t, uint8_t * plane, unsigned int stride)
{
    for (int i=0; i<4*3; i++)
    {
        t[i] *= 255.0;
        t[i] = std::round(t[i]);
        if (t[i] < 0) t[i] = 0;
        if (t[i] > 255) t[i] = 255;
    }
    for (int i=0; i<3; i++)
    {
        for (int j=0; j<3; j++)
        {
            plane[j*stride + i] = t[i*4+j];
        }
    }
}
#endif 

#ifdef USE_SSE2
void FSRCNN::m_integerize4(__m128 * t, uint8_t * plane, unsigned int stride)
{
    float c255 = 255;
    __m128 zero = _mm_setzero_ps();
    __m128 v255 = _mm_load1_ps(&c255);
    t[0] = _mm_mul_ps(t[0], v255);
    t[0] = _mm_max_ps(t[0], zero);
    t[0] = _mm_min_ps(t[0], v255);
    t[1] = _mm_mul_ps(t[1], v255);
    t[1] = _mm_max_ps(t[1], zero);
    t[1] = _mm_min_ps(t[1], v255);
    t[2] = _mm_mul_ps(t[2], v255);
    t[2] = _mm_max_ps(t[2], zero);
    t[2] = _mm_min_ps(t[2], v255);
    t[3] = _mm_mul_ps(t[3], v255);
    t[3] = _mm_max_ps(t[3], zero);
    t[3] = _mm_min_ps(t[3], v255);
    __m128i intvalue;
    uint32_t pixels;
    uint8_t * pix = ((uint8_t*)&pixels);
    intvalue = _mm_cvtps_epi32(t[0]);
    intvalue = _mm_packs_epi32(intvalue,intvalue);
    intvalue = _mm_packus_epi16(intvalue,intvalue);
    pixels = _mm_cvtsi128_si32(intvalue);
    plane[0*stride+0] = pix[0];
    plane[1*stride+0] = pix[1];
    plane[2*stride+0] = pix[2];
    plane[3*stride+0] = pix[3];
    intvalue = _mm_cvtps_epi32(t[1]);
    intvalue = _mm_packs_epi32(intvalue,intvalue);
    intvalue = _mm_packus_epi16(intvalue,intvalue);
    pixels = _mm_cvtsi128_si32(intvalue);
    plane[0*stride+1] = pix[0];
    plane[1*stride+1] = pix[1];
    plane[2*stride+1] = pix[2];
    plane[3*stride+1] = pix[3];
    intvalue = _mm_cvtps_epi32(t[2]);
    intvalue = _mm_packs_epi32(intvalue,intvalue);
    intvalue = _mm_packus_epi16(intvalue,intvalue);
    pixels = _mm_cvtsi128_si32(intvalue);
    plane[0*stride+2] = pix[0];
    plane[1*stride+2] = pix[1];
    plane[2*stride+2] = pix[2];
    plane[3*stride+2] = pix[3];
    intvalue = _mm_cvtps_epi32(t[3]);
    intvalue = _mm_packs_epi32(intvalue,intvalue);
    intvalue = _mm_packus_epi16(intvalue,intvalue);
    pixels = _mm_cvtsi128_si32(intvalue);
    plane[0*stride+3] = pix[0];
    plane[1*stride+3] = pix[1];
    plane[2*stride+3] = pix[2];
    plane[3*stride+3] = pix[3];
}

#else    
void FSRCNN::m_integerize4(float * t, uint8_t * plane, unsigned int stride)
{
    for (int i=0; i<4*4; i++)
    {
        t[i] *= 255.0;
        t[i] = std::round(t[i]);
        if (t[i] < 0) t[i] = 0;
        if (t[i] > 255) t[i] = 255;
    }
    for (int i=0; i<4; i++)
    {
        for (int j=0; j<4; j++)
        {
            plane[j*stride + i] = t[i*4+j];
        }
    }
}
#endif 

