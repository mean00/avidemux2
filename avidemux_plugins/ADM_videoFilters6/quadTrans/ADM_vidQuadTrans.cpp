/***************************************************************************
                          QuadTrans filter ported from frei0r
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
#include "quadTrans.h"
#include "quadTrans_desc.cpp"
#include "ADM_vidQuadTrans.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

extern uint8_t DIA_getQuadTrans(quadTrans *param, ADM_coreVideoFilter *in);


// Add the hook to make it valid plugin
//DECLARE_VIDEO_FILTER(   ADMVideoQuadTrans,   // Class
DECLARE_VIDEO_FILTER_PARTIALIZABLE(   ADMVideoQuadTrans,   // Class
                                      1,0,0,              // Version
                                      ADM_UI_TYPE_BUILD,         // UI
                                      VF_TRANSFORM,            // Category
                                      "quadTrans",            // internal name (must be uniq!)
                                      QT_TRANSLATE_NOOP("quadTrans","Quadrilateral transformation"),            // Display name
                                      QT_TRANSLATE_NOOP("quadTrans","Four point transform.") // Description
                                  );

/**
    \fn QuadTransCreateBuffers
*/
void ADMVideoQuadTrans::QuadTransCreateBuffers(int w, int h, quadTrans_buffers_t * buffers)
{
    buffers->prevparam.algo = 9999;  // invalidate
    buffers->imgCopy = new ADMImageDefault(w,h);

    buffers->integerMap = new int [w*h*2+2];
    buffers->fractionalMap = new int [w*h*2+2];
    buffers->integerMapUV = new int [(w/2)*(h/2)*2+2];
    buffers->fractionalMapUV = new int [(w/2)*(h/2)*2+2];
    
    buffers->bicubicWeights = new int [257*4];
    for (int i=0; i<=256; i++)
    {
        float tmp;
        tmp = 1.0 + i/256.0;    buffers->bicubicWeights[i*4+0] = ((-0.75*(tmp-5.0)*tmp-6.0)*tmp+3.0)*256.0 + 0.5;
        tmp = tmp - 1.0;        buffers->bicubicWeights[i*4+1] = ((1.25*tmp-2.25)*tmp*tmp+1.0)*256.0 + 0.5;
        tmp = 1.0 - tmp;        buffers->bicubicWeights[i*4+2] = ((1.25*tmp-2.25)*tmp*tmp+1.0)*256.0 + 0.5;
                                buffers->bicubicWeights[i*4+3] = 256 - buffers->bicubicWeights[i*4+0] - buffers->bicubicWeights[i*4+1] - buffers->bicubicWeights[i*4+2];
    }
    
    buffers->threads = ADM_cpu_num_processors();
    if (buffers->threads < 1)
        buffers->threads = 1;
    if (buffers->threads > 64)
        buffers->threads = 64;
    buffers->threads /= 2;
    if (buffers->threads < 1)
        buffers->threads = 1;
    buffers->threadsUV = buffers->threads/2;
    if (buffers->threadsUV < 1)
        buffers->threadsUV = 1;

    buffers->worker_threads = new pthread_t [buffers->threads + 2*buffers->threadsUV];
    buffers->worker_thread_args = new worker_thread_arg [buffers->threads + 2*buffers->threadsUV];
}
/**
    \fn QuadTransDestroyBuffers
*/
void ADMVideoQuadTrans::QuadTransDestroyBuffers(quadTrans_buffers_t * buffers)
{
    delete buffers->imgCopy;
    delete [] buffers->integerMap;
    delete [] buffers->fractionalMap;
    delete [] buffers->integerMapUV;
    delete [] buffers->fractionalMapUV;
    delete [] buffers->bicubicWeights;
    delete [] buffers->worker_threads;
    delete [] buffers->worker_thread_args;
}


/**
    \fn bilinear
*/
inline void ADMVideoQuadTrans::bilinear(int w, int h, int stride, uint8_t * in, int x, int y, int fx, int fy, uint8_t * out)
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
inline void ADMVideoQuadTrans::bicubic(int w, int h, int stride, uint8_t * in, int x, int y, int fx, int fy, int * weights, uint8_t * out)
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
void * ADMVideoQuadTrans::worker_thread( void *ptr )
{
    worker_thread_arg * arg = (worker_thread_arg*)ptr;
    int w = arg->w;
    int h = arg->h;
    int ystart = arg->ystart;
    int yincr = arg->yincr;
    int algo = arg->algo;
    int * integerMap = arg->integerMap;
    int * fractionalMap = arg->fractionalMap;
    int stride = arg->stride;
    uint8_t * in = arg->in;
    uint8_t * out = arg->out;
    int * bicubicWeights = arg->bicubicWeights;
    uint8_t blackLevel = arg->blackLevel;

    {
        for (int y=ystart; y<h; y+=yincr)
        {
            for (int x=0; x<w; x++)
            {
                int ix=integerMap[2*(w*y+x)];
                int iy=integerMap[2*(w*y+x)+1];
                int fx=fractionalMap[2*(w*y+x)];
                int fy=fractionalMap[2*(w*y+x)+1];
                
                if (ix>=0)
                {
                    switch(algo) {
                        default:
                        case 0:
                                bilinear(w, h, stride, in, ix, iy, fx, fy, out + x + y*stride);
                            break;
                        case 1:
                                bicubic(w, h, stride, in, ix, iy, fx, fy, bicubicWeights, out + x + y*stride);
                            break;
                    }
                } else {
                    out[x + y*stride] =  blackLevel;
                }
            }
        }
        
    }
    
    pthread_exit(NULL);
    return NULL;
}


/**
    \fn QuadTransProcess_C
*/
void ADMVideoQuadTrans::QuadTransProcess_C(ADMImage *img, int w, int h, quadTrans param, quadTrans_buffers_t * buffers)
{
    if (!img || !buffers || !buffers->imgCopy || !buffers->integerMap || !buffers->fractionalMap || !buffers->integerMapUV || !buffers->fractionalMapUV) return;
    uint8_t * line;

    if (param.algo > 1) param.algo = 1;
    unsigned int algo = param.algo;
    double xs[4],ys[4];
    xs[0] = param.dx1;
    ys[0] = param.dy1;
    xs[1] = param.dx2 + w;
    ys[1] = param.dy2;
    xs[2] = param.dx3;
    ys[2] = param.dy3 + h;
    xs[3] = param.dx4 + w;
    ys[3] = param.dy4 + h;
    
    double midx, midy;
    midx = w-1;
    midx /= 2.0;
    midy = h-1;
    midy /= 2.0;
    
    for (int i=0; i<4; i++)
    {
        xs[i] = (xs[i]-midx)*param.zoom + midx;
        ys[i] = (ys[i]-midy)*param.zoom + midy;
    }


    if (memcmp(&(buffers->prevparam), &param, sizeof(quadTrans)))
    {
        double a,b,c,d,e,f,g,j,a2,b2,c2,u,v,aa,bb,de,sde,v1,v2,u1,u2;
        de=0.0;v1=1000.0;v2=1000.0;
        for (int y=0; y<h; y++)
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
                    int ui = floor(u);
                    int vi = floor(v);
                    int uf = (u-floor(u))*256.0 + 0.5;
                    int vf = (v-floor(v))*256.0 + 0.5;
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
                    buffers->integerMap[2*(y*w+x)]=ui;
                    buffers->integerMap[2*(y*w+x)+1]=vi;
                    buffers->fractionalMap[2*(y*w+x)]=uf;
                    buffers->fractionalMap[2*(y*w+x)+1]=vf;
                }
                else
                {
                    buffers->integerMap[2*(y*w+x)]=-1;
                    buffers->integerMap[2*(y*w+x)+1]=-1;
                }
            
            }
        }
        
        de=0.0;v1=1000.0;v2=1000.0;
        int wh = w/2;
        int hh = h/2;
        for (int y=0; y<hh; y++)
        {
            for (int x=0; x<wh; x++)
            {
                a=xs[0]/2.0-x;
                b=xs[1]/2.0-xs[0]/2.0;
                c=xs[2]/2.0-xs[0]/2.0;
                d=xs[3]/2.0-xs[1]/2.0-(xs[2]/2.0-xs[0]/2.0);
                
                e=ys[0]/2.0-y;
                f=ys[1]/2.0-ys[0]/2.0;
                g=ys[2]/2.0-ys[0]/2.0;
                j=ys[3]/2.0-ys[1]/2.0-(ys[2]/2.0-ys[0]/2.0);
                
                a2=g*d-j*c; b2=e*d-f*c-j*a+g*b; c2=e*b-f*a;
                
                if ((fabs(a2*c2*c2/(b2*b2*b2))< 0.1/wh) && (fabs(a2)<1.0))
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
                    u *= wh;
                    v *= hh;
                    int ui = floor(u);
                    int vi = floor(v);
                    int uf = (u-floor(u))*256.0 + 0.5;
                    int vf = (v-floor(v))*256.0 + 0.5;
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
                    if (ui >= wh-1)
                    {
                        ui = wh-2;
                        uf = 255;
                    }
                    if (vi >= hh-1)
                    {
                        vi = hh-2;
                        vf = 255;
                    }
                    buffers->integerMapUV[2*(y*wh+x)]=ui;
                    buffers->integerMapUV[2*(y*wh+x)+1]=vi;
                    buffers->fractionalMapUV[2*(y*wh+x)]=uf;
                    buffers->fractionalMapUV[2*(y*wh+x)+1]=vf;
                }
                else
                {
                    buffers->integerMapUV[2*(y*wh+x)]=-1;
                    buffers->integerMapUV[2*(y*wh+x)+1]=-1;
                }
            
            }
        }

        memcpy(&(buffers->prevparam), &param, sizeof(quadTrans));
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
        buffers->worker_thread_args[totaltr].integerMap = buffers->integerMap;
        buffers->worker_thread_args[totaltr].fractionalMap = buffers->fractionalMap;
        buffers->worker_thread_args[totaltr].blackLevel = 0;
        buffers->worker_thread_args[totaltr].stride = strides[0];
        buffers->worker_thread_args[totaltr].in = rplanes[0];
        buffers->worker_thread_args[totaltr].out = wplanes[0];
        buffers->worker_thread_args[totaltr].bicubicWeights = buffers->bicubicWeights;
        totaltr++;
    }

    for (int p=1; p<3; p++)
    {
        for (int tr=0; tr<buffers->threadsUV; tr++)
        {
            buffers->worker_thread_args[totaltr].w = w/2;
            buffers->worker_thread_args[totaltr].h = h/2;
            buffers->worker_thread_args[totaltr].ystart = tr;
            buffers->worker_thread_args[totaltr].yincr = buffers->threadsUV;
            buffers->worker_thread_args[totaltr].algo = algo;
            buffers->worker_thread_args[totaltr].integerMap = buffers->integerMapUV;
            buffers->worker_thread_args[totaltr].fractionalMap = buffers->fractionalMapUV;
            buffers->worker_thread_args[totaltr].blackLevel = 128;
            buffers->worker_thread_args[totaltr].stride = strides[p];
            buffers->worker_thread_args[totaltr].in = rplanes[p];
            buffers->worker_thread_args[totaltr].out = wplanes[p];
            buffers->worker_thread_args[totaltr].bicubicWeights = buffers->bicubicWeights;
            totaltr++;
        }
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
bool ADMVideoQuadTrans::configure()
{
    uint8_t r=0;

    r=  DIA_getQuadTrans(&_param, previousFilter);
    if(r) update();
    return r;
}
/**
    \fn getConfiguration
*/

const char   *ADMVideoQuadTrans::getConfiguration(void)
{
    static char s[256];
    const char * algo=NULL;
    switch(_param.algo) {
        default:
        case 0: algo="Bilinear";
            break;
        case 1: algo="Bicubic";
            break;
        case 2: algo="Lanczos";
            break;
    }
    snprintf(s,255,"%s interpolation, Zoom: %.02f, Transform: [%.02f,%.02f], [%.02f,%.02f], [%.02f,%.02f], [%.02f,%.02f]", algo, _param.zoom, _param.dx1, _param.dy1, _param.dx2, _param.dy2, _param.dx3, _param.dy3, _param.dx4, _param.dy4);

    return s;
}
/**
    \fn ctor
*/
ADMVideoQuadTrans::ADMVideoQuadTrans(  ADM_coreVideoFilter *in,CONFcouple *couples)  :ADM_coreVideoFilter(in,couples)
{
    if(!couples || !ADM_paramLoad(couples,quadTrans_param,&_param))
    {
        // Default value
        _param.dx1=0;
        _param.dy1=0;
        _param.dx2=0;
        _param.dy2=0;
        _param.dx3=0;
        _param.dy3=0;
        _param.dx4=0;
        _param.dy4=0;
        _param.zoom=1;
        _param.algo = 0;
    }
    
    QuadTransCreateBuffers(info.width,info.height, &(_buffers));
    update();
}
/**
    \fn update
*/
void ADMVideoQuadTrans::update(void)
{
}
/**
    \fn dtor
*/
ADMVideoQuadTrans::~ADMVideoQuadTrans()
{
    QuadTransDestroyBuffers(&(_buffers));
}
/**
    \fn getCoupledConf
*/
bool ADMVideoQuadTrans::getCoupledConf(CONFcouple **couples)
{
    return ADM_paramSave(couples, quadTrans_param,&_param);
}
/**
    \fn setCoupledConf
*/
void ADMVideoQuadTrans::setCoupledConf(CONFcouple *couples)
{
    ADM_paramLoad(couples, quadTrans_param, &_param);
}



/**
    \fn getNextFrame
    \brief
*/
bool ADMVideoQuadTrans::getNextFrame(uint32_t *fn,ADMImage *image)
{
    if(!previousFilter->getNextFrame(fn,image)) return false;

    QuadTransProcess_C(image,info.width,info.height,_param, &(_buffers));
 
    return 1;
}

