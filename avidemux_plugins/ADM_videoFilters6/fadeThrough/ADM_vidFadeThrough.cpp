/***************************************************************************
                          FadeThrough filter
    Algorithm:
        Copyright Mario Klingemann
        Copyright Maxim Shemanarev
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
#include "ADM_vidMisc.h"
#include "fadeThrough.h"
#include "fadeThrough_desc.cpp"
#include "ADM_vidFadeThrough.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif



#if defined(FADETHROUGH) + defined(FADEIN) + defined(FADEOUT) != 1
#error FADE DIRECTION NOT DEFINED!
#endif

extern uint8_t DIA_getFadeThrough(fadeThrough *param, ADM_coreVideoFilter *in);

/**
    \fn FadeThroughCreateBuffers
*/
void ADMVideoFadeThrough::FadeThroughCreateBuffers(int w, int h, fadeThrough_buffers_t * buffers)
{
    for (int i=0; i<3; i++)
        buffers->lut[i] = new uint8_t [256];
    
    buffers->blendRGBcached = ~0;
    buffers->vignetteRGBcached = ~0;
    
    buffers->rgbBufStride = ADM_IMAGE_ALIGN(w * 4);
    buffers->rgbBufRaw = new ADM_byteBuffer();
    if (buffers->rgbBufRaw)
        buffers->rgbBufRaw->setSize(buffers->rgbBufStride * h);
    buffers->convertYuvToRgb = new ADMColorScalerFull(ADM_CS_BICUBIC,w,h,w,h,ADM_COLOR_YV12,ADM_COLOR_RGB32A);
    buffers->convertRgbToYuv = new ADMColorScalerFull(ADM_CS_BICUBIC,w,h,w,h,ADM_COLOR_RGB32A,ADM_COLOR_YV12);
    buffers->rgbBufImage = new ADMImageRef(w,h);
    if (buffers->rgbBufImage)
    {
        buffers->rgbBufImage->_colorspace = ADM_COLOR_RGB32A;
        buffers->rgbBufImage->_planes[0] = buffers->rgbBufRaw->at(0);
        buffers->rgbBufImage->_planes[1] = buffers->rgbBufImage->_planes[2] = NULL;
        buffers->rgbBufImage->_planeStride[0] = buffers->rgbBufStride;
        buffers->rgbBufImage->_planeStride[1] = buffers->rgbBufImage->_planeStride[2] = 0;
    }
    buffers->blurStack = new uint32_t [512];

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

    buffers->qtr_worker_threads = new pthread_t [buffers->threads + buffers->threadsUV];
    buffers->qtr_worker_thread_args = new qtr_worker_thread_arg [buffers->threads + buffers->threadsUV];
    
}
/**
    \fn FadeThroughDestroyBuffers
*/
void ADMVideoFadeThrough::FadeThroughDestroyBuffers(fadeThrough_buffers_t * buffers)
{
    for (int i=0; i<3; i++)
        delete [] buffers->lut[i];

    if (buffers->convertYuvToRgb) delete buffers->convertYuvToRgb;
    if (buffers->convertRgbToYuv) delete buffers->convertRgbToYuv;
    if (buffers->rgbBufRaw) buffers->rgbBufRaw->clean();
    if (buffers->rgbBufImage) delete buffers->rgbBufImage;
    if (buffers->rgbBufRaw) delete buffers->rgbBufRaw;
    delete [] buffers->blurStack;

    delete buffers->imgCopy;
    delete [] buffers->bicubicWeights;
    delete [] buffers->qtr_worker_threads;
    delete [] buffers->qtr_worker_thread_args;
}


/**
    \fn bilinear
*/
inline void ADMVideoFadeThrough::bilinear(int w, int h, int stride, uint8_t * in, int x, int y, int fx, int fy, uint8_t * out)
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
inline void ADMVideoFadeThrough::bicubic(int w, int h, int stride, uint8_t * in, int x, int y, int fx, int fy, int * weights, uint8_t * out)
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
    \fn qtr_worker_thread
*/
void * ADMVideoFadeThrough::qtr_worker_thread( void *ptr )
{
    qtr_worker_thread_arg * arg = (qtr_worker_thread_arg*)ptr;
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
    \fn FadeThroughProcess_C
*/
void ADMVideoFadeThrough::FadeThroughProcess_C(ADMImage *img, int w, int h, fadeThrough param, fadeThrough_buffers_t * buffers)
{
    if (!img || !buffers || !buffers->imgCopy) return;
    if (!buffers->bicubicWeights || !buffers->qtr_worker_threads || !buffers->qtr_worker_thread_args) return;
    if (!buffers->rgbBufRaw || !buffers->rgbBufImage || !buffers->convertYuvToRgb || !buffers->convertRgbToYuv || !buffers->blurStack) return;

    uint32_t startMs = param.startTime;
    uint32_t endMs = param.endTime;

    if (startMs > endMs)
    {
        uint64_t tmp = endMs;
        endMs = startMs;
        startMs = tmp;
    }
    
    uint32_t imgMs = img->Pts/1000LL;
    
    if (startMs == endMs)
        return;
    if (imgMs < startMs)
        return;
    if (imgMs > endMs)
        return;

    double frac = ((double)(imgMs - startMs)) / ((double)(endMs - startMs));


    if (param.enableBright || param.enableSat || param.enableBlend)
    {
        for (int i=0; i<3; i++)
            for (int j=0; j<256; j++)
                buffers->lut[i][j] = j;
    }

    if (param.enableBright)
    {
        double level = TransientPoint(frac, param.transientBright, param.transientDurationBright);
        level *= (param.peakBright-1.0);	// -1 .. +1
        if(img->_range == ADM_COL_RANGE_MPEG)
            level *= 220.0;	// +-220
        else
            level *= 255.0;	// +-255
        
        int px;
        int intLevel = level + 0.49;
        for (int j=0; j<256; j++)
        {
            px = buffers->lut[0][j];
            px += intLevel;
            if(img->_range == ADM_COL_RANGE_MPEG)
            {
                if (px < 16) px = 16;
                if (px > 235) px = 235;
            } else {
                if (px < 0) px = 0;
                if (px > 255) px = 255;
            }
            buffers->lut[0][j] = px;
        }
    }

    if (param.enableSat)
    {
        double level = TransientPoint(frac, param.transientSat, param.transientDurationSat);
        level *= (param.peakSat-1.0);
        level += 1;	// 0..2
        int satMul = level * 65536.0 + 0.49;
        int px;
        for (int i=1; i<3; i++)
        {
            for (int j=0; j<256; j++)
            {
                px = buffers->lut[i][j];
                px -= 128;
                px *= satMul;
                px /= 65536;
                px += 128;
                if(img->_range == ADM_COL_RANGE_MPEG)
                {
                    if (px < 16) px = 16;
                    if (px > 240) px = 240;
                } else {
                    if (px < 0) px = 0;
                    if (px > 255) px = 255;
                }
                buffers->lut[i][j] = px;
            }
        }
    }
    
    if (param.enableBlend)
    {
        double level = TransientPoint(frac, param.transientBlend, param.transientDurationBlend);
        level *= param.peakBlend;

        if ((buffers->blendRGBcached != (param.rgbColorBlend & 0x00FFFFFF)) || ((img->_range == ADM_COL_RANGE_MPEG) ^ buffers->blendRangeCached))
        {
            // rgb to yuv:
            int rgb[3], yuv[3];
            rgb[0] = (param.rgbColorBlend>>16)&0xFF;
            rgb[1] = (param.rgbColorBlend>>8)&0xFF;
            rgb[2] = (param.rgbColorBlend>>0)&0xFF;
            if(img->_range == ADM_COL_RANGE_MPEG)
            {
                yuv[0] = std::round( 0.257*rgb[0] + 0.504*rgb[1] + 0.098*rgb[2]) + 16;
                yuv[1] = std::round(-0.148*rgb[0] - 0.291*rgb[1] + 0.439*rgb[2]) + 128;
                yuv[2] = std::round( 0.439*rgb[0] - 0.368*rgb[1] - 0.071*rgb[2]) + 128;
                for (int i=0; i<3; i++)
                    if (yuv[i] <   16) yuv[i] = 16;
                if (yuv[0] > 235) yuv[0] = 235;
                if (yuv[1] > 240) yuv[1] = 240;
                if (yuv[2] > 240) yuv[2] = 240;
            } else {
                yuv[0] = std::round( 0.299*rgb[0] + 0.587*rgb[1] + 0.114*rgb[2]);
                yuv[1] = std::round(-0.169*rgb[0] - 0.331*rgb[1] + 0.500*rgb[2]) + 128;
                yuv[2] = std::round( 0.500*rgb[0] - 0.419*rgb[1] - 0.081*rgb[2]) + 128;
                for (int i=0; i<3; i++)
                {
                    if (yuv[i] <   0) yuv[i] = 0;
                    if (yuv[i] > 255) yuv[i] = 255;
                }
            }
            
            // fix swapped uv
            int uvswap = yuv[1]; yuv[1] = yuv[2]; yuv[2]=uvswap;
            
            buffers->blendRGBcached = param.rgbColorBlend & 0x00FFFFFF;
            buffers->blendRangeCached = (img->_range == ADM_COL_RANGE_MPEG);
            for (int i=0; i<3; i++)
                buffers->blendYUVcached[i] = yuv[i];
        }
        
        int imgblend = (1.0-level)*256.0 + 0.49;
        int px;
        for (int i=0; i<3; i++)
        {
            int yuvi = buffers->blendYUVcached[i]*level*256.0 + 0.49;
            for (int j=0; j<256; j++)
            {
                px = imgblend*buffers->lut[i][j] + yuvi;
                buffers->lut[i][j] = px / 256;
            }
        }
    }
    
    if (param.enableBright || param.enableSat || param.enableBlend)    // apply lut
    {
        uint8_t * imgPlanes[3];
        int imgStrides[3];

        img->GetWritePlanes(imgPlanes);
        img->GetPitches(imgStrides);

        for (int p=0; p<3; p++)
        {
            int width = ((p==0) ? w:(w/2));
            int height = ((p==0) ? h:(h/2));
            int px;
            uint8_t * ipl = imgPlanes[p];
            for (int y=0; y<height; y++)
            {
                for (int x=0; x<width; x++)
                {
                    ipl[x] = buffers->lut[p][ipl[x]];
                }
                ipl += imgStrides[p];
            }
        }
    }

    if (param.enableBlur)
    {
        double level = TransientPoint(frac, param.transientBlur, param.transientDurationBlur);

        unsigned int radius = level*param.peakBlur;
        if (radius > 254) radius = 254;
        if (radius >= 1)
        {
            buffers->convertYuvToRgb->convertImage(img,buffers->rgbBufRaw->at(0));

            int x, y;
            uint8_t * rgbPtr = buffers->rgbBufRaw->at(0);
            for(y = 0; y < h; y++)
            {
                StackBlurLine_C((rgbPtr + y*buffers->rgbBufStride), w, 4, buffers->blurStack, radius);
            };

            for(x = 0; x < w; x++)
            {
                StackBlurLine_C((rgbPtr + x*4), h, buffers->rgbBufStride, buffers->blurStack, radius);
            };

            buffers->convertRgbToYuv->convertImage(buffers->rgbBufImage,img);
        }
    }


    if (param.enableRot || param.enableZoom)
    {
        double rot_level = TransientPoint(frac, param.transientRot, param.transientDurationRot);
        double zoom_level = TransientPoint(frac, param.transientZoom, param.transientDurationZoom);
        rot_level *= param.peakRot;
        rot_level *= M_PI/180.0;
        param.peakZoom -= 1;
        zoom_level *= param.peakZoom;
        if (!param.enableRot)
            rot_level = 0.0;
        if (!param.enableZoom)
            zoom_level = 0.0;
        zoom_level += 1;

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
        
        double newx,newy;
        for (int i=0; i<4; i++)
        {
            xs[i] -= midx;
            ys[i] -= midy;
            newx = std::cos(rot_level)*xs[i] - std::sin(rot_level)*ys[i];
            newy = std::sin(rot_level)*xs[i] + std::cos(rot_level)*ys[i];
            newx *= zoom_level;
            newy *= zoom_level;
            newx += midx;
            newy += midy;
            xs[i] = newx;
            ys[i] = newy;
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
            buffers->qtr_worker_thread_args[totaltr].w = w;
            buffers->qtr_worker_thread_args[totaltr].h = h;
            buffers->qtr_worker_thread_args[totaltr].ystart = tr;
            buffers->qtr_worker_thread_args[totaltr].yincr = buffers->threads;
            buffers->qtr_worker_thread_args[totaltr].algo = 1;
            buffers->qtr_worker_thread_args[totaltr].xs = xs;
            buffers->qtr_worker_thread_args[totaltr].ys = ys;
            buffers->qtr_worker_thread_args[totaltr].blackLevel = 0;
            buffers->qtr_worker_thread_args[totaltr].stride = strides[0];
            buffers->qtr_worker_thread_args[totaltr].in = rplanes[0];
            buffers->qtr_worker_thread_args[totaltr].in2 = NULL;
            buffers->qtr_worker_thread_args[totaltr].out = wplanes[0];
            buffers->qtr_worker_thread_args[totaltr].out2 = NULL;
            buffers->qtr_worker_thread_args[totaltr].bicubicWeights = buffers->bicubicWeights;
            totaltr++;
        }

        for (int tr=0; tr<buffers->threadsUV; tr++)
        {
            buffers->qtr_worker_thread_args[totaltr].w = w/2;
            buffers->qtr_worker_thread_args[totaltr].h = h/2;
            buffers->qtr_worker_thread_args[totaltr].ystart = tr;
            buffers->qtr_worker_thread_args[totaltr].yincr = buffers->threadsUV;
            buffers->qtr_worker_thread_args[totaltr].algo = 1;
            buffers->qtr_worker_thread_args[totaltr].xs = xsh;
            buffers->qtr_worker_thread_args[totaltr].ys = ysh;
            buffers->qtr_worker_thread_args[totaltr].blackLevel = 128;
            buffers->qtr_worker_thread_args[totaltr].stride = strides[1];
            buffers->qtr_worker_thread_args[totaltr].in = rplanes[1];
            buffers->qtr_worker_thread_args[totaltr].in2 = rplanes[2];
            buffers->qtr_worker_thread_args[totaltr].out = wplanes[1];
            buffers->qtr_worker_thread_args[totaltr].out2 = wplanes[2];
            buffers->qtr_worker_thread_args[totaltr].bicubicWeights = buffers->bicubicWeights;
            totaltr++;
        }

        for (int tr=0; tr<totaltr; tr++)
        {
            pthread_create( &buffers->qtr_worker_threads[tr], NULL, qtr_worker_thread, (void*) &buffers->qtr_worker_thread_args[tr]);
        }
        // work in thread workers...
        for (int tr=0; tr<totaltr; tr++)
        {
            pthread_join( buffers->qtr_worker_threads[tr], NULL);
        }

    }


    if (param.enableVignette)
    {
        double level = TransientPoint(frac, param.transientVignette, param.transientDurationVignette);

        if ((buffers->vignetteRGBcached != (param.rgbColorVignette & 0x00FFFFFF)) || ((img->_range == ADM_COL_RANGE_MPEG) ^ buffers->vignetteRangeCached))
        {
            // rgb to yuv:
            int rgb[3], yuv[3];
            rgb[0] = (param.rgbColorVignette>>16)&0xFF;
            rgb[1] = (param.rgbColorVignette>>8)&0xFF;
            rgb[2] = (param.rgbColorVignette>>0)&0xFF;
            if(img->_range == ADM_COL_RANGE_MPEG)
            {
                yuv[0] = std::round( 0.257*rgb[0] + 0.504*rgb[1] + 0.098*rgb[2]) + 16;
                yuv[1] = std::round(-0.148*rgb[0] - 0.291*rgb[1] + 0.439*rgb[2]) + 128;
                yuv[2] = std::round( 0.439*rgb[0] - 0.368*rgb[1] - 0.071*rgb[2]) + 128;
                for (int i=0; i<3; i++)
                    if (yuv[i] <   16) yuv[i] = 16;
                if (yuv[0] > 235) yuv[0] = 235;
                if (yuv[1] > 240) yuv[1] = 240;
                if (yuv[2] > 240) yuv[2] = 240;
            } else {
                yuv[0] = std::round( 0.299*rgb[0] + 0.587*rgb[1] + 0.114*rgb[2]);
                yuv[1] = std::round(-0.169*rgb[0] - 0.331*rgb[1] + 0.500*rgb[2]) + 128;
                yuv[2] = std::round( 0.500*rgb[0] - 0.419*rgb[1] - 0.081*rgb[2]) + 128;
                for (int i=0; i<3; i++)
                {
                    if (yuv[i] <   0) yuv[i] = 0;
                    if (yuv[i] > 255) yuv[i] = 255;
                }
            }
            
            // fix swapped uv
            int uvswap = yuv[1]; yuv[1] = yuv[2]; yuv[2]=uvswap;

            buffers->vignetteRGBcached = param.rgbColorVignette & 0x00FFFFFF;
            buffers->vignetteRangeCached = (img->_range == ADM_COL_RANGE_MPEG);
            for (int i=0; i<3; i++)
                buffers->vignetteYUVcached[i] = yuv[i];
        }
            
        uint8_t * imgPlanes[3];
        int imgStrides[3];

        img->GetWritePlanes(imgPlanes);
        img->GetPitches(imgStrides);

        int imgblend,yuvblend;
        for (int p=0; p<3; p++)
        {
            int width = ((p==0) ? w:(w/2));
            int height = ((p==0) ? h:(h/2));
            int px;
            uint8_t * ipl = imgPlanes[p];
            
            double corner = ((double)(width/2))*(width/2) + ((double)(height/2))*(height/2);
            double vec;
            
            corner *= param.peakVignette+(1.0-level);
            
            for (int y=0; y<height; y++)
            {
                for (int x=0; x<width; x++)
                {
                    vec = ((double)(x-width/2))*(x-width/2) + ((double)(y-height/2))*(y-height/2);
                    if (vec > corner)
                    {
                        imgblend = 0;
                        yuvblend = 256;
                    } else
                    if ((corner - vec) > 256.0*256.0)
                    {
                        imgblend = 256;
                        yuvblend = 0;
                    } else
                    {
                        imgblend = std::sqrt(corner - vec);
                        yuvblend = 256-imgblend;
                    }
                    
                    px = imgblend*ipl[x] + yuvblend*buffers->vignetteYUVcached[p];
                    ipl[x] = px / 256;
                }
                ipl += imgStrides[p];
            }
        }
        
    }
}

/**
    \fn TransientPoint
*/
double ADMVideoFadeThrough::TransientPoint(double frac, int transient, double duration)
{
    if (duration == 0.0)
        return 1.0;

#if defined(FADETHROUGH)
    if ((frac > duration) && (frac < (1.0-duration)))
        return 1.0;
    
    if (frac >= 0.5)
        frac = 1.0-frac;
#endif

#if defined(FADEIN) || defined(FADEOUT)
  #if defined(FADEIN)
    frac = 1.0 - frac;
  #endif
    if (frac > duration)
        return 1.0;
#endif

    frac /= duration;
    
    switch (transient)
    {
        case 0:		// raised cosine
            return (1.0 - (std::cos(M_PI*(frac))/2.0 + 0.5));
        case 1:		// ramp
            return frac;
        case 2:		// quadratic
            return (frac*frac);
        case 3:		// inverse quadratic
            return std::sqrt(frac);
        case 4:		// exponential
            #define EXP_OFFSET	(4.0)
            return ((std::exp(frac*EXP_OFFSET)-1.0)/(std::exp(EXP_OFFSET)-1.0));
        default:
            return 1.0;
    
    }
}


static const uint16_t g_stack_blur8_mul[255] = 
    {
        512,512,456,512,328,456,335,512,405,328,271,456,388,335,292,512,
        454,405,364,328,298,271,496,456,420,388,360,335,312,292,273,512,
        482,454,428,405,383,364,345,328,312,298,284,271,259,496,475,456,
        437,420,404,388,374,360,347,335,323,312,302,292,282,273,265,512,
        497,482,468,454,441,428,417,405,394,383,373,364,354,345,337,328,
        320,312,305,298,291,284,278,271,265,259,507,496,485,475,465,456,
        446,437,428,420,412,404,396,388,381,374,367,360,354,347,341,335,
        329,323,318,312,307,302,297,292,287,282,278,273,269,265,261,512,
        505,497,489,482,475,468,461,454,447,441,435,428,422,417,411,405,
        399,394,389,383,378,373,368,364,359,354,350,345,341,337,332,328,
        324,320,316,312,309,305,301,298,294,291,287,284,281,278,274,271,
        268,265,262,259,257,507,501,496,491,485,480,475,470,465,460,456,
        451,446,442,437,433,428,424,420,416,412,408,404,400,396,392,388,
        385,381,377,374,370,367,363,360,357,354,350,347,344,341,338,335,
        332,329,326,323,320,318,315,312,310,307,304,302,299,297,294,292,
        289,287,285,282,280,278,275,273,271,269,267,265,263,261,259
    };

static const uint8_t g_stack_blur8_shr[255] = 
    {
          9, 11, 12, 13, 13, 14, 14, 15, 15, 15, 15, 16, 16, 16, 16, 17, 
         17, 17, 17, 17, 17, 17, 18, 18, 18, 18, 18, 18, 18, 18, 18, 19, 
         19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 20, 20, 20,
         20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 21,
         21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21,
         21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 22, 22, 22, 22, 22, 22, 
         22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22,
         22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 23, 
         23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23,
         23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23,
         23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 
         23, 23, 23, 23, 23, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 
         24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
         24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
         24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
         24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24
    };
/**
    \fn StackBlurLine_C
*/
void ADMVideoFadeThrough::StackBlurLine_C(uint8_t * line, int len, int pixPitch, uint32_t * stack, unsigned int radius)
{
    uint_fast32_t div;
    uint_fast32_t mul_sum;
    uint_fast32_t shr_sum;

    uint_fast32_t l, p, i;
    uint_fast32_t stack_ptr;
    uint_fast32_t stack_start;

    const uint8_t * src_pix_ptr;
          uint8_t * dst_pix_ptr;
    uint8_t *     stack_pix_ptr;

    uint_fast32_t sum_r = 0;
    uint_fast32_t sum_g = 0;
    uint_fast32_t sum_b = 0;
    uint_fast32_t sum_in_r = 0;
    uint_fast32_t sum_in_g = 0;
    uint_fast32_t sum_in_b = 0;
    uint_fast32_t sum_out_r = 0;
    uint_fast32_t sum_out_g = 0;
    uint_fast32_t sum_out_b = 0;

    uint_fast32_t lm  = len - 1;

    if ((radius > 0) && (len > 1))
    {
        div = radius * 2 + 1;
        mul_sum = g_stack_blur8_mul[radius];
        shr_sum = g_stack_blur8_shr[radius];

        for(i = 0; i <= radius; i++)
        {
            if ((radius - i) > lm)
                src_pix_ptr = line + pixPitch*lm;
            else
                src_pix_ptr = line + pixPitch*(radius - i);    // fix flickering at the T/L edges, by reflecting the image backward
            stack_pix_ptr    = (uint8_t *)&stack[i];
            *(uint32_t*)stack_pix_ptr = *(uint32_t*)src_pix_ptr;
            sum_out_r       += src_pix_ptr[0];
            sum_r           += src_pix_ptr[0] * (i + 1);
            sum_out_g       += src_pix_ptr[1];
            sum_g           += src_pix_ptr[1] * (i + 1);
            sum_out_b       += src_pix_ptr[2];
            sum_b           += src_pix_ptr[2] * (i + 1);
        }
        src_pix_ptr = line;
        for(i = 1; i <= radius; i++)
        {
            if(i <= lm) src_pix_ptr += pixPitch; 
            stack_pix_ptr = (uint8_t *)&stack[i + radius];
            *(uint32_t*)stack_pix_ptr = *(uint32_t*)src_pix_ptr;
            sum_in_r        += src_pix_ptr[0];
            sum_r           += src_pix_ptr[0] * (radius + 1 - i);
            sum_in_g        += src_pix_ptr[1];
            sum_g           += src_pix_ptr[1] * (radius + 1 - i);
            sum_in_b        += src_pix_ptr[2];
            sum_b           += src_pix_ptr[2] * (radius + 1 - i);
        }

        stack_ptr = radius;
        p = radius;
        if(p > lm) p = lm;
        src_pix_ptr = line + pixPitch*p;
        dst_pix_ptr = line;
        for(l = 0; l < len; l++)
        {
            dst_pix_ptr[0] = (sum_r * mul_sum) >> shr_sum;
            dst_pix_ptr[1] = (sum_g * mul_sum) >> shr_sum;
            dst_pix_ptr[2] = (sum_b * mul_sum) >> shr_sum;
            dst_pix_ptr   += pixPitch;

            sum_r -= sum_out_r;
            sum_g -= sum_out_g;
            sum_b -= sum_out_b;
   
            stack_start = stack_ptr + div - radius;
            if(stack_start >= div) stack_start -= div;
            stack_pix_ptr = (uint8_t *)&stack[stack_start];

            sum_out_r -= stack_pix_ptr[0];
            sum_out_g -= stack_pix_ptr[1];
            sum_out_b -= stack_pix_ptr[2];

            if(p < lm) 
                src_pix_ptr += pixPitch;
            else if(p < lm*2) 
                src_pix_ptr -= pixPitch;    // fix flickering at the B/R edges, by reflecting the image backward
            ++p;

            *(uint32_t*)stack_pix_ptr = *(uint32_t*)src_pix_ptr;

            sum_in_r += src_pix_ptr[0];
            sum_in_g += src_pix_ptr[1];
            sum_in_b += src_pix_ptr[2];
            sum_r    += sum_in_r;
            sum_g    += sum_in_g;
            sum_b    += sum_in_b;

            ++stack_ptr;
            if(stack_ptr >= div) stack_ptr = 0;
            stack_pix_ptr = (uint8_t *)&stack[stack_ptr];

            sum_in_r  -= stack_pix_ptr[0];
            sum_out_r += stack_pix_ptr[0];
            sum_in_g  -= stack_pix_ptr[1];
            sum_out_g += stack_pix_ptr[1];
            sum_in_b  -= stack_pix_ptr[2];
            sum_out_b += stack_pix_ptr[2];
        }
    }
}

/**
    \fn IsFadeIn
*/
bool ADMVideoFadeThrough::IsFadeIn()
{
#if defined(FADEIN)
    return true;
#else
    return false;
#endif
}

/**
    \fn IsFadeOut
*/
bool ADMVideoFadeThrough::IsFadeOut()
{
#if defined(FADEOUT)
    return true;
#else
    return false;
#endif
}

/**
    \fn configure
*/
bool ADMVideoFadeThrough::configure()
{
    uint8_t r=0;

    r=  DIA_getFadeThrough(&_param, previousFilter);
    if(r) update();
    return r;
}
/**
    \fn getConfiguration
*/

const char   *ADMVideoFadeThrough::getConfiguration(void)
{
    static char s[1024];
    char startTimeStr[128];
    char endTimeStr[128];
    snprintf(startTimeStr,127,"%s",ADM_us2plain(_param.startTime*1000LL));
    snprintf(endTimeStr,127,"%s",ADM_us2plain(_param.endTime*1000LL));

    snprintf(s,1023,"%s - %s: ",startTimeStr,endTimeStr);
    
    bool first = true;
    if (_param.enableBright)
    {
        if (!first)
            strcat(s," + ");
        strcat(s,"Brightness");
        first = false;
    }
    
    if (_param.enableSat)
    {
        if (!first)
            strcat(s," + ");
        strcat(s,"Saturation");
        first = false;
    }
    
    if (_param.enableBlend)
    {
        if (!first)
            strcat(s," + ");
        strcat(s,"Color blend");
        first = false;
    }
    
    if (_param.enableBlur)
    {
        if (!first)
            strcat(s," + ");
        strcat(s,"Blur");
        first = false;
    }
    
    if (_param.enableRot)
    {
        if (!first)
            strcat(s," + ");
        strcat(s,"Rotation");
        first = false;
    }
    
    if (_param.enableZoom)
    {
        if (!first)
            strcat(s," + ");
        strcat(s,"Zoom");
        first = false;
    }
    
    if (_param.enableVignette)
    {
        if (!first)
            strcat(s," + ");
        strcat(s,"Vignette");
        first = false;
    }

    if (first)
        strcat(s,"NO EFFECT");

    return s;
}
/**
    \fn ctor
*/
ADMVideoFadeThrough::ADMVideoFadeThrough(  ADM_coreVideoFilter *in,CONFcouple *couples)  :ADM_coreVideoFilter(in,couples)
{
    if(!couples || !ADM_paramLoad(couples,fadeThrough_param,&_param))
    {
        // Default value
        _param.startTime = info.markerA / 1000LL;
        _param.endTime = info.markerB / 1000LL;
        _param.enableBright = false;
        _param.enableSat = false;
        _param.enableBlend = false;
        _param.enableBlur = false;
        _param.enableRot = false;
        _param.enableZoom = false;
        _param.enableVignette = false;
        _param.rgbColorBlend = 0;
        _param.rgbColorVignette = 0;
        _param.peakBright = 1.0;
        _param.peakSat = 1.0;
        _param.peakBlend = 1.0;
        _param.peakBlur = 0;
        _param.peakRot = 0;
        _param.peakZoom = 1.0;
        _param.peakVignette = 0;
        _param.transientBright = 0;
        _param.transientSat = 0;
        _param.transientBlend = 0;
        _param.transientBlur = 0;
        _param.transientRot = 0;
        _param.transientZoom = 0;
        _param.transientVignette = 0;
        
        float defaultTrD = 0.5;
#if defined(FADEIN) || defined(FADEOUT)
        defaultTrD = 1.0;
#endif
        _param.transientDurationBright = defaultTrD;
        _param.transientDurationSat = defaultTrD;
        _param.transientDurationBlend = defaultTrD;
        _param.transientDurationBlur = defaultTrD;
        _param.transientDurationRot = defaultTrD;
        _param.transientDurationZoom = defaultTrD;
        _param.transientDurationVignette = defaultTrD;
    }
    
    FadeThroughCreateBuffers(info.width,info.height, &(_buffers));
    update();
}
/**
    \fn update
*/
void ADMVideoFadeThrough::update(void)
{
}
/**
    \fn dtor
*/
ADMVideoFadeThrough::~ADMVideoFadeThrough()
{
    FadeThroughDestroyBuffers(&(_buffers));
}
/**
    \fn getCoupledConf
*/
bool ADMVideoFadeThrough::getCoupledConf(CONFcouple **couples)
{
    return ADM_paramSave(couples, fadeThrough_param,&_param);
}
/**
    \fn setCoupledConf
*/
void ADMVideoFadeThrough::setCoupledConf(CONFcouple *couples)
{
    ADM_paramLoad(couples, fadeThrough_param, &_param);
}



/**
    \fn getNextFrame
    \brief
*/
bool ADMVideoFadeThrough::getNextFrame(uint32_t *fn,ADMImage *image)
{
    if(!previousFilter->getNextFrame(fn,image)) return false;

    FadeThroughProcess_C(image,info.width,info.height,_param, &(_buffers));
 
    return 1;
}

