/***************************************************************************
                          ImageStab filter
    Algorithm:
        Copyright 2010 Marko Cebokli
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
#include "imageStab.h"
#include "imageStab_desc.cpp"
#include "ADM_vidImageStab.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

extern uint8_t DIA_getImageStab(imageStab *param, ADM_coreVideoFilter *in);


// Add the hook to make it valid plugin
DECLARE_VIDEO_FILTER(   ADMVideoImageStab,   // Class
                                      1,0,0,              // Version
                                      ADM_UI_TYPE_BUILD,         // UI
                                      VF_TRANSFORM,            // Category
                                      "imageStab",            // internal name (must be uniq!)
                                      QT_TRANSLATE_NOOP("imageStab","Image stabilizer"),            // Display name
                                      QT_TRANSLATE_NOOP("imageStab","Reduce camera shakiness.") // Description
                                  );

/**
    \fn ImageStabCreateBuffers
*/
void ADMVideoImageStab::ImageStabCreateBuffers(int w, int h, imageStab_buffers_t * buffers)
{
    buffers->prevPts = ADM_NO_PTS;
    buffers->imgCopy = new ADMImageDefault(w,h);
    
    buffers->bicubicWeights = new int [257*4];
    for (int i=0; i<=256; i++)
    {
        float tmp;
        tmp = 1.0 + i/256.0;    buffers->bicubicWeights[i*4+0] = ((-0.75*(tmp-5.0)*tmp-6.0)*tmp+3.0)*256.0 + 0.5;
        tmp = tmp - 1.0;        buffers->bicubicWeights[i*4+1] = ((1.25*tmp-2.25)*tmp*tmp+1.0)*256.0 + 0.5;
        tmp = 1.0 - tmp;        buffers->bicubicWeights[i*4+2] = ((1.25*tmp-2.25)*tmp*tmp+1.0)*256.0 + 0.5;
                                buffers->bicubicWeights[i*4+3] = 256 - buffers->bicubicWeights[i*4+0] - buffers->bicubicWeights[i*4+1] - buffers->bicubicWeights[i*4+2];
    }
    
    buffers->prevChromaHist[0] = -1.0;  // invalidate
    
    buffers->motestp = new motest(w,h, MOTION_ESTIMATION_CONTRAST_THRESHOLD);

    buffers->threads = ADM_cpu_num_processors();
    if (buffers->threads < 1)
        buffers->threads = 1;
    if (buffers->threads > 64)
        buffers->threads = 64;
    buffers->threadsUV = buffers->threads/4;
    if (buffers->threadsUV < 1)
        buffers->threadsUV = 1;
    buffers->threads -= buffers->threadsUV;
    if (buffers->threads < 1)
        buffers->threads = 1;

    buffers->worker_threads = new pthread_t [buffers->threads + buffers->threadsUV];
    buffers->worker_thread_args = new worker_thread_arg [buffers->threads + buffers->threadsUV];
}
/**
    \fn ImageStabDestroyBuffers
*/
void ADMVideoImageStab::ImageStabDestroyBuffers(imageStab_buffers_t * buffers)
{
    delete buffers->imgCopy;
    delete [] buffers->bicubicWeights;
    delete buffers->motestp;
    delete [] buffers->worker_threads;
    delete [] buffers->worker_thread_args;
}


/**
    \fn bilinear
*/
inline void ADMVideoImageStab::bilinear(int w, int h, int stride, uint8_t * in, int x, int y, int fx, int fy, uint8_t * out)
{
    int i,a,b,k,l,k1,l1;
    
    k  = y*stride+x;
    k1 = k  + 1;
    l  = k  + stride;
    l1 = k1 + stride;
    
    a=in[k]*256+(in[k1]-in[k])*fx;
    b=in[l]*256+(in[l1]-in[l])*fx;
    out[0]=(a*256+(b-a)*fy)/65536;
}

/**
    \fn bicubic
*/
inline void ADMVideoImageStab::bicubic(int w, int h, int stride, uint8_t * in, int x, int y, int fx, int fy, int * weights, uint8_t * out)
{
    int m = x-1;
    int n = y-1;
    if (m < 0)
        m = 0;
    if (n < 0)
        n = 0;
    if (m+5 > w)
        m = w-4;
    if (n+5 > h)
        n = h-4;

    int * wx, * wy;
    wx = weights + fx*4;
    wy = weights + fy*4;
    
    int b,i,k,l, p, pp;
    k = n*stride + m;

    l = k;
    p  = wx[0] * in[l+0];
    p += wx[1] * in[l+1];
    p += wx[2] * in[l+2];
    p += wx[3] * in[l+3];
    pp = wy[0]*p;
    l += stride;
    p  = wx[0] * in[l+0];
    p += wx[1] * in[l+1];
    p += wx[2] * in[l+2];
    p += wx[3] * in[l+3];
    pp += wy[1]*p;
    l += stride;
    p  = wx[0] * in[l+0];
    p += wx[1] * in[l+1];
    p += wx[2] * in[l+2];
    p += wx[3] * in[l+3];
    pp += wy[2]*p;
    l += stride;
    p  = wx[0] * in[l+0];
    p += wx[1] * in[l+1];
    p += wx[2] * in[l+2];
    p += wx[3] * in[l+3];
    pp += wy[3]*p;
    
    pp /= 65536;
    if (pp < 0)
        pp = 0;
    if (pp > 255)
        pp = 255;
    out[0] = pp;
}



/**
    \fn worker_thread
*/
void * ADMVideoImageStab::worker_thread( void *ptr )
{
    worker_thread_arg * arg = (worker_thread_arg*)ptr;
    int w = arg->w;
    int h = arg->h;
    int ystart = arg->ystart;
    int yincr = arg->yincr;
    int algo = arg->algo;
    double * xs = arg->xs;
    double * ys = arg->ys;
    int stride = arg->stride;
    uint8_t * in = arg->in;
    uint8_t * in2 = arg->in2;
    uint8_t * out = arg->out;
    uint8_t * out2 = arg->out2;
    int * bicubicWeights = arg->bicubicWeights;
    uint8_t blackLevel = arg->blackLevel;
    
    {
        double a,b,c,d,e,f,g,j,a2,b2,c2,u,v,aa,bb,de,sde,v1,v2,u1,u2;
        de=0.0;v1=1000.0;v2=1000.0;
        for (int y=ystart; y<h; y+=yincr)
        {
            for (int x=0; x<w; x++)
            {
                a=xs[0]-x;
                b=xs[1]-xs[0];
                c=xs[2]-xs[0];
                d=xs[3]-xs[1]-(xs[2]-xs[0]);
                
                e=ys[0]-y;
                f=ys[1]-ys[0];
                g=ys[2]-ys[0];
                j=ys[3]-ys[1]-(ys[2]-ys[0]);
                
                a2=g*d-j*c; b2=e*d-f*c-j*a+g*b; c2=e*b-f*a;
                
                if ((fabs(a2*c2*c2/(b2*b2*b2))< 0.1/w) && (fabs(a2)<1.0))
                {
                    v1 = (b2!=0.0) ? -c2/b2 : 1000.0;
                    v2=1000.0;
                }
                else
                {
                    de=b2*b2-4.0*a2*c2;
                    if (de>=0.0)
                    {
                        sde=sqrt(de);
                        v1=(-b2+sde)/2.0/a2;
                        v2=(-b2-sde)/2.0/a2;
                    }
                    else
                    {
                        v1=1001.0;
                        v2=1001.0;
                    }
                }
                aa=b+d*v1; bb=f+j*v1;
                if (fabs(aa)>fabs(bb))
                    u1 = (aa!=0.0) ? -(a+c*v1)/aa : 1000.0;
                else
                    u1 = (bb!=0.0) ? -(e+g*v1)/bb : 1000.0;
                aa=b+d*v2; bb=f+j*v2;
                if (fabs(aa)>fabs(bb))
                    u2 = (aa!=0.0) ? -(a+c*v2)/aa : 1000.0;
                else
                    u2 = (bb!=0.0) ? -(e+g*v2)/bb : 1000.0;
                
                if ((u1>=0.0)&&(u1<1.0)&&(v1>=0.0)&&(v1<1.0))
                {
                    u=u1;
                    v=v1;
                }
                else
                {
                    if ((u2>=0.0)&&(u2<1.0)&&(v2>=0.0)&&(v2<1.0))
                    {
                        u=u2;
                        v=v2;
                    }
                    else
                    {
                        u=1002.0;
                        v=1002.0;
                    }
                }
                
                
                if ((u>=0.0)&&(u<1.0)&&(v>=0.0)&&(v<1.0))
                {
                    u *= w;
                    v *= h;
                    int ui = std::floor(u);
                    int vi = std::floor(v);
                    int uf = (u-std::floor(u))*256.0 + 0.5;
                    int vf = (v-std::floor(v))*256.0 + 0.5;
                    if (uf > 255)
                    {
                        ui++;
                        uf = 0;
                    }
                    if (vf > 255)
                    {
                        vi++;
                        vf = 0;
                    }
                    if (ui >= w-1)
                    {
                        ui = w-2;
                        uf = 255;
                    }
                    if (vi >= h-1)
                    {
                        vi = h-2;
                        vf = 255;
                    }

                    switch(algo) {
                        default:
                        case 0:
                                bilinear(w, h, stride, in, ui, vi, uf, vf, out + x + y*stride);
                                if (in2 && out2)
                                    bilinear(w, h, stride, in2, ui, vi, uf, vf, out2 + x + y*stride);
                            break;
                        case 1:
                                bicubic(w, h, stride, in, ui, vi, uf, vf, bicubicWeights, out + x + y*stride);
                                if (in2 && out2)
                                    bicubic(w, h, stride, in2, ui, vi, uf, vf, bicubicWeights, out2 + x + y*stride);
                            break;
                    }

                }
                else
                {
                    out[x + y*stride] =  blackLevel;
                    if (in2 && out2)
                        out2[x + y*stride] =  blackLevel;
                }
            
            }
        }
        
    }
    
    pthread_exit(NULL);
    return NULL;
}

/**
    \fn ImageStabProcess_C
*/
void ADMVideoImageStab::ImageStabProcess_C(ADMImage *img, int w, int h, imageStab param, imageStab_buffers_t * buffers, bool * newScene, float * sceneDiff)
{
    if (!img || !buffers || !buffers->imgCopy) return;
    if (!buffers->bicubicWeights || !buffers->motestp || !buffers->worker_threads || !buffers->worker_thread_args) return;
    uint8_t * line;

    if (param.algo > 1) param.algo = 1;
    if (param.motionEstimation > 1) param.motionEstimation = 1;
    if (param.smoothing < 0.0) param.smoothing = 0.0;
    if (param.smoothing > 1.0) param.smoothing = 1.0;
    if (param.gravity < 0.0) param.gravity = 0.0;
    if (param.gravity > 1.0) param.gravity = 1.0;
    if (param.sceneThreshold < 0.0) param.sceneThreshold = 0.0;
    if (param.sceneThreshold > 1.0) param.sceneThreshold = 1.0;

    unsigned int algo = param.algo;
    unsigned int motionEstimation = param.motionEstimation;
    float sceneThreshold = param.sceneThreshold;
    double smoothing = param.smoothing;
    double gravity = param.gravity;
    
    int ystride,ustride,vstride;
    uint8_t * yptr, * uptr, * vptr;
    float currChromaHist[64];
    bool scdet = false;
    int i,j,x,y;
    
    bool sameImage = (buffers->prevPts == img->Pts);
    buffers->prevPts = img->Pts;

    memset(currChromaHist, 0, 64*sizeof(float));
    
    if (buffers->prevChromaHist[0] < 0.0)
        scdet = true;

    // UV planes: get histograms
    ustride=img->GetPitch(PLANAR_U);
    uptr=img->GetWritePtr(PLANAR_U);
    vstride=img->GetPitch(PLANAR_V);
    vptr=img->GetWritePtr(PLANAR_V);
    for(y=0;y<h/2;y++)	// 4:2:0
    {
        for (x=0;x<w/2;x++)
        {
            currChromaHist[uptr[x]/8 +  0]++;
            currChromaHist[vptr[x]/8 + 32]++;
        }
        uptr+=ustride;
        vptr+=vstride;
    }
    
    if (!scdet)
    {
        float sum = 0.0;
        for(i=0; i<64; i++)
        {
            sum += fabs(currChromaHist[i] - buffers->prevChromaHist[i]);
        }
        sum /= ((h/2) * (w/2));
        sum=std::sqrt(sum/2);

        if ((sceneThreshold < 1.0) && (sum > sceneThreshold))    // disable detect, if sceneThreshold set to max
            scdet = true;

        if (sceneDiff) *sceneDiff = sum;
    }

    memcpy(buffers->prevChromaHist, currChromaHist, 64*sizeof(float));

    if (newScene) *newScene= scdet;
    
    if (sameImage)
    {
        if (newScene) *newScene= buffers->newSceneSameImage;
        if (sceneDiff) *sceneDiff = buffers->sceneDiffSameImage;
    } else {
        if (newScene) buffers->newSceneSameImage = *newScene;
        if (sceneDiff) buffers->sceneDiffSameImage = *sceneDiff;
    }

    if (!sameImage)
        buffers->motestp->addNextImage(scdet ? NULL : img);

    double xs[4],ys[4];
    xs[0] = 0;
    ys[0] = 0;
    xs[1] = w-1;
    ys[1] = 0;
    xs[2] = 0;
    ys[2] = h-1;
    xs[3] = w-1;
    ys[3] = h-1;
    
    double midx, midy;
    midx = w-1;
    midx /= 2.0;
    midy = h-1;
    midy /= 2.0;

    if (!scdet)
    {
        if (!sameImage)
            buffers->motestp->estimateMotion(motionEstimation);

        double globalMotion[2];
        double rotation;

        buffers->motestp->getMotionParameters(globalMotion, &rotation);

        double alpha;
        double decay, decayr;
        // smoothing [0 .. 0.5 .. 1] -~> [0.5 .. 0.1 .. 0.001]
        alpha = 0.5 - std::sqrt(smoothing)/2.0;
        if (alpha < 0.001)
            alpha = 0.001;
        if (param.autoGravity)
        {
            double hdisp,vdisp;
            hdisp = buffers->last[0]/w;
            vdisp = buffers->last[1]/h;
            hdisp *= 4.0;
            vdisp *= 4.0;
            double displacement = std::sqrt(hdisp*hdisp + vdisp*vdisp);
            decay = 1.0 - displacement;
            if (decay < 0.0) decay = 0.0;
            decay = decay*decay;
            if (decay > 0.99)
                decay = 0.99;
            
            decayr = 1.0 - std::fabs(buffers->last[2] * 2.0);
            if (decayr < 0.0) decayr = 0.0;
            decayr = decayr*decayr;
            if (decayr > 0.99)
                decayr = 0.99;
        } else {
            // gravity   [0 .. 0.5 .. 1] -~> [0.99 .. 0.9 .. 0.5]
            decay = 1.0 - (gravity*gravity*gravity)/2.0;
            if (decay > 0.99)
                decay = 0.99;
            decayr = decay;
        }
        
        // filter
        if (!sameImage)
        {
            buffers->hist[0] = alpha*globalMotion[0] + (1.0 - alpha)*buffers->hist[0];
            buffers->hist[1] = alpha*globalMotion[1] + (1.0 - alpha)*buffers->hist[1];
            buffers->hist[2] = alpha*rotation + (1.0 - alpha)*buffers->hist[2];
        }
        
        globalMotion[0] -= buffers->hist[0];
        globalMotion[1] -= buffers->hist[1];
        rotation -= buffers->hist[2];
        
        globalMotion[0] *= -1.0;
        globalMotion[1] *= -1.0;
        rotation *= -1.0;
        
        if (sameImage)
        {
            globalMotion[0] += buffers->lastSameImage[0];
            globalMotion[1] += buffers->lastSameImage[1];
            rotation += buffers->lastSameImage[2];
        } else {
            globalMotion[0] += buffers->last[0];
            globalMotion[1] += buffers->last[1];
            rotation += buffers->last[2];
        }
        
        if (!sameImage)
        {
            memcpy(buffers->lastSameImage, buffers->last, sizeof(double)*3);
            buffers->last[0] = globalMotion[0] * decay;
            buffers->last[1] = globalMotion[1] * decay;
            buffers->last[2] = rotation * decayr;
        }
        
        double newx,newy;
        newx = std::cos(rotation)*globalMotion[0] - std::sin(rotation)*globalMotion[1];
        newy = std::sin(rotation)*globalMotion[0] + std::cos(rotation)*globalMotion[1];
        globalMotion[0] = newx;
        globalMotion[1] = newy;

        for (i=0; i<4; i++)
        {
            xs[i] -= midx;
            ys[i] -= midy;
            newx = std::cos(rotation)*xs[i] - std::sin(rotation)*ys[i];
            newy = std::sin(rotation)*xs[i] + std::cos(rotation)*ys[i];
            newx += midx;
            newy += midy;
            newx += globalMotion[0];
            newy += globalMotion[1];
            xs[i] = newx;
            ys[i] = newy;
        }
    }
    else
    {
        memset(buffers->hist, 0, sizeof(double)*3);
        memset(buffers->last, 0, sizeof(double)*3);
        memset(buffers->lastSameImage, 0, sizeof(double)*3);
    }
    
    
    for (int i=0; i<4; i++)
    {
        xs[i] = (xs[i]-midx)*param.zoom + midx;
        ys[i] = (ys[i]-midy)*param.zoom + midy;
    }

    double xsh[4],ysh[4];
    for (int i=0; i<4; i++)
    {
        xsh[i] = xs[i]/2.0;
        ysh[i] = ys[i]/2.0;
    }

    uint8_t * rplanes[3];
    uint8_t * wplanes[3];
    int strides[3];

    buffers->imgCopy->duplicate(img);
    buffers->imgCopy->GetPitches(strides);
    buffers->imgCopy->GetWritePlanes(rplanes);
    img->GetWritePlanes(wplanes);
    
    int totaltr = 0;
    
    for (int tr=0; tr<buffers->threads; tr++)
    {
        buffers->worker_thread_args[totaltr].w = w;
        buffers->worker_thread_args[totaltr].h = h;
        buffers->worker_thread_args[totaltr].ystart = tr;
        buffers->worker_thread_args[totaltr].yincr = buffers->threads;
        buffers->worker_thread_args[totaltr].algo = algo;
        buffers->worker_thread_args[totaltr].xs = xs;
        buffers->worker_thread_args[totaltr].ys = ys;
        buffers->worker_thread_args[totaltr].blackLevel = 0;
        buffers->worker_thread_args[totaltr].stride = strides[0];
        buffers->worker_thread_args[totaltr].in = rplanes[0];
        buffers->worker_thread_args[totaltr].in2 = NULL;
        buffers->worker_thread_args[totaltr].out = wplanes[0];
        buffers->worker_thread_args[totaltr].out2 = NULL;
        buffers->worker_thread_args[totaltr].bicubicWeights = buffers->bicubicWeights;
        totaltr++;
    }

    for (int tr=0; tr<buffers->threadsUV; tr++)
    {
        buffers->worker_thread_args[totaltr].w = w/2;
        buffers->worker_thread_args[totaltr].h = h/2;
        buffers->worker_thread_args[totaltr].ystart = tr;
        buffers->worker_thread_args[totaltr].yincr = buffers->threadsUV;
        buffers->worker_thread_args[totaltr].algo = algo;
        buffers->worker_thread_args[totaltr].xs = xsh;
        buffers->worker_thread_args[totaltr].ys = ysh;
        buffers->worker_thread_args[totaltr].blackLevel = 128;
        buffers->worker_thread_args[totaltr].stride = strides[1];
        buffers->worker_thread_args[totaltr].in = rplanes[1];
        buffers->worker_thread_args[totaltr].in2 = rplanes[2];
        buffers->worker_thread_args[totaltr].out = wplanes[1];
        buffers->worker_thread_args[totaltr].out2 = wplanes[2];
        buffers->worker_thread_args[totaltr].bicubicWeights = buffers->bicubicWeights;
        totaltr++;
    }

    for (int tr=0; tr<totaltr; tr++)
    {
        pthread_create( &buffers->worker_threads[tr], NULL, worker_thread, (void*) &buffers->worker_thread_args[tr]);
    }
    // work in thread workers...
    for (int tr=0; tr<totaltr; tr++)
    {
        pthread_join( buffers->worker_threads[tr], NULL);
    }

}

/**
    \fn configure
*/
bool ADMVideoImageStab::configure()
{
    uint8_t r=0;

    r=  DIA_getImageStab(&_param, previousFilter);
    if(r) update();
    return r;
}
/**
    \fn getConfiguration
*/

const char   *ADMVideoImageStab::getConfiguration(void)
{
    static char s[512];
    const char * algo=NULL;
    const char * motionEstimation=NULL;
    switch(_param.algo) {
        default:
        case 0: algo="Bilinear";
            break;
        case 1: algo="Bicubic";
            break;
        case 2: algo="Lanczos";
            break;
    }
    
    switch(_param.motionEstimation) {
        default:
        case 0: motionEstimation="Accurate";
            break;
        case 1: motionEstimation="Fast";
            break;
    }
    
    char grav[16];
    if (_param.autoGravity)
        strcpy(grav, "auto");
    else
        snprintf(grav,15,"%.2f",_param.gravity);

    snprintf(s,511,"Smoothing: %.2f, Gravity: %s, Scene threshold: %.2f, %s interpolation, Zoom: %.02f, %s motion estimation", _param.smoothing, grav, _param.sceneThreshold, algo, _param.zoom, motionEstimation);

    return s;
}
/**
    \fn ctor
*/
ADMVideoImageStab::ADMVideoImageStab(  ADM_coreVideoFilter *in,CONFcouple *couples)  :ADM_coreVideoFilter(in,couples)
{
    if(!couples || !ADM_paramLoad(couples,imageStab_param,&_param))
    {
        // Default value
        _param.smoothing=0.5;
        _param.gravity=0.5;
        _param.autoGravity=true;
        _param.sceneThreshold=0.5;
        _param.zoom=1;
        _param.algo = 0;
        _param.motionEstimation = 0;
    }
    
    ImageStabCreateBuffers(info.width,info.height, &(_buffers));
    update();
}
/**
    \fn update
*/
void ADMVideoImageStab::update(void)
{
}
/**
    \fn dtor
*/
ADMVideoImageStab::~ADMVideoImageStab()
{
    ImageStabDestroyBuffers(&(_buffers));
}
/**
    \fn getCoupledConf
*/
bool ADMVideoImageStab::getCoupledConf(CONFcouple **couples)
{
    return ADM_paramSave(couples, imageStab_param,&_param);
}
/**
    \fn setCoupledConf
*/
void ADMVideoImageStab::setCoupledConf(CONFcouple *couples)
{
    ADM_paramLoad(couples, imageStab_param, &_param);
}



/**
    \fn getNextFrame
    \brief
*/
bool ADMVideoImageStab::getNextFrame(uint32_t *fn,ADMImage *image)
{
    if(!previousFilter->getNextFrame(fn,image)) return false;

    ImageStabProcess_C(image,info.width,info.height,_param, &(_buffers), NULL, NULL);
 
    return 1;
}

