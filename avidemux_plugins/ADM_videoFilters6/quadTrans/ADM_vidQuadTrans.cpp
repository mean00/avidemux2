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
    buffers->rgbBufStride = ADM_IMAGE_ALIGN(w * 4);
    buffers->rgbBufRawIn = new ADM_byteBuffer();
    if (buffers->rgbBufRawIn)
        buffers->rgbBufRawIn->setSize(buffers->rgbBufStride * h);
    buffers->rgbBufRawOut = new ADM_byteBuffer();
    if (buffers->rgbBufRawOut)
        buffers->rgbBufRawOut->setSize(buffers->rgbBufStride * h);
    buffers->convertYuvToRgb = new ADMColorScalerFull(ADM_CS_BICUBIC,w,h,w,h,ADM_COLOR_YV12,ADM_COLOR_RGB32A);
    buffers->convertRgbToYuv = new ADMColorScalerFull(ADM_CS_BICUBIC,w,h,w,h,ADM_COLOR_RGB32A,ADM_COLOR_YV12);
    buffers->rgbBufImage = new ADMImageRef(w,h);
    if (buffers->rgbBufImage)
    {
        buffers->rgbBufImage->_colorspace = ADM_COLOR_RGB32A;
        buffers->rgbBufImage->_planes[0] = buffers->rgbBufRawOut->at(0);
        buffers->rgbBufImage->_planes[1] = buffers->rgbBufImage->_planes[2] = NULL;
        buffers->rgbBufImage->_planeStride[0] = buffers->rgbBufStride;
        buffers->rgbBufImage->_planeStride[1] = buffers->rgbBufImage->_planeStride[2] = 0;
    }
    
    buffers->integerMap = new int [w*h*2+2];
    buffers->fractionalMap = new int [w*h*2+2];
    
    buffers->bicubicWeights = new int [257*4];
    for (int i=0; i<=256; i++)
    {
        float tmp;
        tmp = 1.0 + i/256.0;    buffers->bicubicWeights[i*4+0] = ((-0.75*(tmp-5.0)*tmp-6.0)*tmp+3.0)*256.0 + 0.5;
        tmp = tmp - 1.0;        buffers->bicubicWeights[i*4+1] = ((1.25*tmp-2.25)*tmp*tmp+1.0)*256.0 + 0.5;
        tmp = 1.0 - tmp;        buffers->bicubicWeights[i*4+2] = ((1.25*tmp-2.25)*tmp*tmp+1.0)*256.0 + 0.5;
                                buffers->bicubicWeights[i*4+3] = 256 - buffers->bicubicWeights[i*4+0] - buffers->bicubicWeights[i*4+1] - buffers->bicubicWeights[i*4+2];
    }
}
/**
    \fn QuadTransDestroyBuffers
*/
void ADMVideoQuadTrans::QuadTransDestroyBuffers(quadTrans_buffers_t * buffers)
{
    if (buffers->convertYuvToRgb) delete buffers->convertYuvToRgb;
    if (buffers->convertRgbToYuv) delete buffers->convertRgbToYuv;
    if (buffers->rgbBufRawIn) buffers->rgbBufRawIn->clean();
    if (buffers->rgbBufRawOut) buffers->rgbBufRawOut->clean();
    if (buffers->rgbBufImage) delete buffers->rgbBufImage;
    if (buffers->rgbBufRawIn) delete buffers->rgbBufRawIn;
    if (buffers->rgbBufRawOut) delete buffers->rgbBufRawOut;
    delete [] buffers->integerMap;
    delete [] buffers->fractionalMap;
    delete [] buffers->bicubicWeights;
}


/**
    \fn bilinear
*/
inline void ADMVideoQuadTrans::bilinear(int w, int h, int stride, uint8_t * in, int x, int y, int fx, int fy, uint8_t * out)
{
    int i,a,b,k,l,k1,l1;
    
    k  = y*stride+x*4;
    k1 = k  + 4;
    l  = k  + stride;
    l1 = k1 + stride;
    
    a=in[k+0]*256+(in[k1+0]-in[k+0])*fx;
    b=in[l+0]*256+(in[l1+0]-in[l+0])*fx;
    out[0]=(a*256+(b-a)*fy)/65536;
    
    a=in[k+1]*256+(in[k1+1]-in[k+1])*fx;
    b=in[l+1]*256+(in[l1+1]-in[l+1])*fx;
    out[1]=(a*256+(b-a)*fy)/65536;

    a=in[k+2]*256+(in[k1+2]-in[k+2])*fx;
    b=in[l+2]*256+(in[l1+2]-in[l+2])*fx;
    out[2]=(a*256+(b-a)*fy)/65536;

    /*a=in[k+3]*256+(in[k1+3]-in[k+3])*fx;
    b=in[l+3]*256+(in[l1+3]-in[l+3])*fx;
    out[3]=(a*256+(b-a)*fy)/65536;*/
    out[3]=0xFF;    // alpha channel
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
    k = n*stride + m*4;

    for (b=0; b<3 ;b++)
    {
        l = k;
        p  = wx[0] * in[l+0];
        p += wx[1] * in[l+4];
        p += wx[2] * in[l+8];
        p += wx[3] * in[l+12];
        pp = wy[0]*p;
        l += stride;
        p  = wx[0] * in[l+0];
        p += wx[1] * in[l+4];
        p += wx[2] * in[l+8];
        p += wx[3] * in[l+12];
        pp += wy[1]*p;
        l += stride;
        p  = wx[0] * in[l+0];
        p += wx[1] * in[l+4];
        p += wx[2] * in[l+8];
        p += wx[3] * in[l+12];
        pp += wy[2]*p;
        l += stride;
        p  = wx[0] * in[l+0];
        p += wx[1] * in[l+4];
        p += wx[2] * in[l+8];
        p += wx[3] * in[l+12];
        pp += wy[3]*p;
        
        pp /= 65536;
        if (pp < 0)
            pp = 0;
        if (pp > 255)
            pp = 255;
        out[b] = pp;

        k++;
    }
    
    out[3]=0xFF;    // alpha channel
}

/**
    \fn QuadTransProcess_C
*/
void ADMVideoQuadTrans::QuadTransProcess_C(ADMImage *img, int w, int h, quadTrans param, quadTrans_buffers_t * buffers)
{
    if (!img || !buffers || !buffers->rgbBufRawIn || !buffers->rgbBufRawOut || !buffers->rgbBufImage || !buffers->convertYuvToRgb || !buffers->convertRgbToYuv) return;
    uint8_t * line;

    if (param.algo > 1) param.algo = 1;
    unsigned int algo = param.algo;
    float xs[4],ys[4];
    xs[0] = param.dx1;
    ys[0] = param.dy1;
    xs[1] = param.dx2 + w;
    ys[1] = param.dy2;
    xs[2] = param.dx3;
    ys[2] = param.dy3 + h;
    xs[3] = param.dx4 + w;
    ys[3] = param.dy4 + h;
    
    float midx, midy;
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
        
        memcpy(&(buffers->prevparam), &param, sizeof(quadTrans));
    }


    buffers->convertYuvToRgb->convertImage(img,buffers->rgbBufRawIn->at(0));

    int x,y;
    for (y=0;y<h;y++)
    {
        for (x=0;x<w;x++)
        {
            int ix=buffers->integerMap[2*(w*y+x)];
            int iy=buffers->integerMap[2*(w*y+x)+1];
            int fx=buffers->fractionalMap[2*(w*y+x)];
            int fy=buffers->fractionalMap[2*(w*y+x)+1];

            if (ix>=0)
            {
                if ((fx == 0) && (fy == 0))
                {
                    memcpy(buffers->rgbBufRawOut->at(0) + x*4 + y*buffers->rgbBufStride, buffers->rgbBufRawIn->at(0) + ix*4 + iy*buffers->rgbBufStride, 4);
                }
                else
                {
                    switch(algo) {
                        default:
                        case 0:
                                bilinear(w, h, buffers->rgbBufStride, buffers->rgbBufRawIn->at(0), ix, iy, fx, fy, buffers->rgbBufRawOut->at(0) + x*4 + y*buffers->rgbBufStride);
                            break;
                        case 1:
                                bicubic(w, h, buffers->rgbBufStride, buffers->rgbBufRawIn->at(0), ix, iy, fx, fy, buffers->bicubicWeights, buffers->rgbBufRawOut->at(0) + x*4 + y*buffers->rgbBufStride);
                            break;
                    }
                }
            } else {
                memset(buffers->rgbBufRawOut->at(0) + x*4 + y*buffers->rgbBufStride, 0, 4);
            }
        }
    }


    buffers->convertRgbToYuv->convertImage(buffers->rgbBufImage,img);

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

