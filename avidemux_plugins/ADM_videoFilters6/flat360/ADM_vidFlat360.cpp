/***************************************************************************
                          Flat360 filter
        Copyright 2019 Eugene Lyapustin
                  2023 szlldm
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
#include "flat360.h"
#include "flat360_desc.cpp"
#include "ADM_vidFlat360.h"

#ifndef M_PI
#define M_PI        3.14159265358979323846
#endif
#ifndef M_PI_2
#define M_PI_2     1.57079632679489661923
#endif
#ifndef M_PI_4
#define M_PI_4     0.78539816339744830962
#endif
#ifndef M_1_PI
#define M_1_PI     0.31830988618379067154
#endif
#ifndef M_2_PI
#define M_2_PI     0.63661977236758134308
#endif

extern uint8_t DIA_getFlat360(flat360 *param, ADM_coreVideoFilter *in);


// Add the hook to make it valid plugin
//DECLARE_VIDEO_FILTER(   ADMVideoFlat360,   // Class
DECLARE_VIDEO_FILTER_PARTIALIZABLE(   ADMVideoFlat360,   // Class
                                      1,0,0,              // Version
                                      ADM_UI_TYPE_BUILD,         // UI
                                      VF_TRANSFORM,            // Category
                                      "flat360",            // internal name (must be uniq!)
                                      QT_TRANSLATE_NOOP("flat360","Flatten 360"),            // Display name
                                      QT_TRANSLATE_NOOP("flat360","Project 360 degree videos.") // Description
                                  );

/**
    \fn Flat360CreateBuffers
*/
void ADMVideoFlat360::Flat360CreateBuffers(int w, int h, flat360_buffers_t * buffers)
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
    \fn Flat360DestroyBuffers
*/
void ADMVideoFlat360::Flat360DestroyBuffers(flat360_buffers_t * buffers)
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
inline void ADMVideoFlat360::bilinear(int w, int h, int stride, uint8_t * in, int x, int y, int fx, int fy, uint8_t * out)
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
inline void ADMVideoFlat360::bicubic(int w, int h, int stride, uint8_t * in, int x, int y, int fx, int fy, int * weights, uint8_t * out)
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
void * ADMVideoFlat360::worker_thread( void *ptr )
{
    worker_thread_arg * arg = (worker_thread_arg*)ptr;
    int w = arg->w;
    int h = arg->h;
    int ystart = arg->ystart;
    int yincr = arg->yincr;
    int algo = arg->algo;
    int * integerMap = arg->integerMap;
    int * fractionalMap = arg->fractionalMap;
    int istride = arg->istride;
    int ostride = arg->ostride;
    uint8_t * in = arg->in;
    uint8_t * out = arg->out;
    int * bicubicWeights = arg->bicubicWeights;

    {
        for (int y=ystart; y<h; y+=yincr)
        {
            for (int x=0; x<w; x++)
            {
                int ix=integerMap[2*(w*y+x)];
                int iy=integerMap[2*(w*y+x)+1];
                int fx=fractionalMap[2*(w*y+x)];
                int fy=fractionalMap[2*(w*y+x)+1];

                switch(algo) {
                    default:
                    case 0:
                            bilinear(w, h, istride, in, ix, iy, fx, fy, out + x + y*ostride);
                        break;
                    case 1:
                            bicubic(w, h, istride, in, ix, iy, fx, fy, bicubicWeights, out + x + y*ostride);
                        break;
                }
            }
        }
        
    }
    
    pthread_exit(NULL);
    return NULL;
}

/**
    \fn createMapping_worker_thread
*/
void * ADMVideoFlat360::createMapping_worker_thread( void *ptr )
{
    worker_thread_arg * arg = (worker_thread_arg*)ptr;
    int width = arg->w;
    int height = arg->h;
    int ystart = arg->ystart;
    int yincr = arg->yincr;
    bool chroma = arg->chroma;
    int method = arg->param.method;
    float degToRad = M_PI/180.0;
    float yaw = arg->param.yaw * degToRad;
    float pitch = arg->param.pitch * degToRad;
    float roll = arg->param.roll * degToRad;
    float fov = arg->param.fov * degToRad;
    float distortion = arg->param.distortion;
    distortion = distortion*distortion/100;
    int pad = arg->param.pad;

    float hFov = -std::tan(fov / 2);
    float vFov = (hFov * height) / width;

    float rmx[9];
    rmx[0] = std::sin(yaw)*std::sin(pitch)*std::sin(roll) - std::cos(yaw)*std::cos(roll);
    rmx[1] = -std::sin(yaw)*std::sin(pitch)*std::cos(roll) - std::cos(yaw)*std::sin(roll);
    rmx[2] = std::sin(yaw)*std::cos(pitch);
    rmx[3] = std::cos(pitch)*std::sin(roll);
    rmx[4] = -std::cos(pitch)*std::cos(roll);
    rmx[5] = -std::sin(pitch);
    rmx[6] = std::cos(yaw)*std::sin(pitch)*std::sin(roll) + std::sin(yaw)*std::cos(roll);
    rmx[7] = -std::cos(yaw)*std::sin(pitch)*std::cos(roll) + std::sin(yaw)*std::sin(roll);
    rmx[8] = std::cos(yaw)*std::cos(pitch);

    for (int y=ystart; y<height; y+=yincr)
    {
        for (int x=0; x<width; x++)
        {
            float dx = (((2.0 * x) / width) - 1.0);
            float dy = (((2.0 * y) / height) - 1.0);
            float vec[3];
            vec[0] = hFov * dx * (1 + distortion*(dx*dx + dy*dy));
            vec[1] = vFov * dy * (1 + distortion*(dx*dx + dy*dy));
            vec[2] = 1;
            float xyz[3];
            xyz[0] = rmx[0]*vec[0] + rmx[1]*vec[1] + rmx[2]*vec[2];
            xyz[1] = rmx[3]*vec[0] + rmx[4]*vec[1] + rmx[5]*vec[2];
            xyz[2] = rmx[6]*vec[0] + rmx[7]*vec[1] + rmx[8]*vec[2];
            float norm = std::sqrt(xyz[0]*xyz[0] + xyz[1]*xyz[1] + xyz[2]*xyz[2]);
            xyz[0] /= norm;
            xyz[1] /= norm;
            xyz[2] /= norm;
            float u,v;
            u = v = 0;
            
            switch (method)
            {
                default:
                case 0: //Equirectangular
                    {
                        float phi   = std::atan2(xyz[0], xyz[2]) / M_PI;
                        float theta = std::asin(xyz[1]) / (M_PI_2);
                        u = (0.5*phi + 0.5)*width;
                        v = (0.5*theta + 0.5)*height;

                        while (u < 0) u += width;
                        while (u >= width) u -= width;

                        if (v < 0) v = 0;
                        if (v > height - 1) v = height - 1;
                    }
                    break;
                case 1: //Equi-Angular Cubemap
                    {
                        float phi   = std::atan2(xyz[0], xyz[2]);
                        float theta = std::asin(xyz[1]);
                        float phi_norm, theta_threshold;
                        int direction = 0;
                        if ((phi >= -M_PI_4) && phi < (M_PI_4)) {
                            direction = FRONT;
                            phi_norm = phi;
                        } else if ((phi >= -(M_PI_2 + M_PI_4)) && (phi < -M_PI_4)) {
                            direction = LEFT;
                            phi_norm = phi + M_PI_2;
                        } else if ((phi >= M_PI_4) && phi < (M_PI_2 + M_PI_4)) {
                            direction = RIGHT;
                            phi_norm = phi - M_PI_2;
                        } else {
                            direction = BACK;
                            phi_norm = phi + ((phi > 0) ? -M_PI : M_PI);
                        }
                        
                        theta_threshold = std::atan(std::cos(phi_norm));
                        if (theta > theta_threshold) {
                            direction = DOWN;
                        } else if (theta < -theta_threshold) {
                            direction = UP;
                        }

                        int u_face = 0;
                        int v_face = 0;
                        switch (direction) {
                            case RIGHT:
                                u = -xyz[2] / xyz[0];
                                v =  xyz[1] / xyz[0];
                                u_face = 2;
                                v_face = 0;
                                break;
                            case LEFT:
                                u = -xyz[2] / xyz[0];
                                v = -xyz[1] / xyz[0];
                                u_face = 0;
                                v_face = 0;
                                break;
                            case UP:
                                u = -xyz[2] / xyz[1];
                                v = xyz[0] / xyz[1];
                                u_face = 2;
                                v_face = 1;
                                break;
                            case DOWN:
                                u = -xyz[2] / xyz[1];
                                v = -xyz[0] / xyz[1];
                                u_face = 0;
                                v_face = 1;
                                break;
                            case FRONT:
                                u =  xyz[0] / xyz[2];
                                v =  xyz[1] / xyz[2];
                                u_face = 1;
                                v_face = 0;
                                break;
                            case BACK:
                                u =  xyz[1] / xyz[2];
                                v =  xyz[0] / xyz[2];
                                u_face = 1;
                                v_face = 1;
                                break;
                            default:
                                break;
                        }
    
                        float pixel_pad = (chroma ? 1:2);
                        float u_pad = pixel_pad / width;
                        float v_pad = pixel_pad / height;

                        u = M_2_PI * std::atan(u) + 0.5;
                        v = M_2_PI * std::atan(v) + 0.5;

                        u = (u + u_face) * (1.0 - 2.0 * u_pad) / 3.0 + u_pad;
                        v = v * (0.5 - 2.0 * v_pad) + v_pad + 0.5 * v_face;

                        u *= width;
                        v *= height;

                        u -= 0.5;
                        v -= 0.5;                        
                    }
                    break;
                case 2: //Cubemap 3x2 RLUDFB
                    {
                        float phi   = std::atan2(xyz[0], xyz[2]);
                        float theta = std::asin(xyz[1]);
                        float phi_norm, theta_threshold;
                        int direction = 0;
                        if ((phi >= -M_PI_4) && phi < (M_PI_4)) {
                            direction = FRONT;
                            phi_norm = phi;
                        } else if ((phi >= -(M_PI_2 + M_PI_4)) && (phi < -M_PI_4)) {
                            direction = LEFT;
                            phi_norm = phi + M_PI_2;
                        } else if ((phi >= M_PI_4) && phi < (M_PI_2 + M_PI_4)) {
                            direction = RIGHT;
                            phi_norm = phi - M_PI_2;
                        } else {
                            direction = BACK;
                            phi_norm = phi + ((phi > 0.f) ? -M_PI : M_PI);
                        }
                        
                        theta_threshold = std::atan(std::cos(phi_norm));
                        if (theta > theta_threshold) {
                            direction = DOWN;
                        } else if (theta < -theta_threshold) {
                            direction = UP;
                        }

                        int u_face = 0;
                        int v_face = 0;
                        switch (direction) {
                            case RIGHT:
                                u = -xyz[2] / xyz[0];
                                v =  xyz[1] / xyz[0];
                                u_face = 0;
                                v_face = 0;
                                break;
                            case LEFT:
                                u = -xyz[2] / xyz[0];
                                v = -xyz[1] / xyz[0];
                                u_face = 1;
                                v_face = 0;
                                break;
                            case UP:
                                u = -xyz[0] / xyz[1];
                                v = -xyz[2] / xyz[1];
                                u_face = 2;
                                v_face = 0;
                                break;
                            case DOWN:
                                u =  xyz[0] / xyz[1];
                                v = -xyz[2] / xyz[1];
                                u_face = 0;
                                v_face = 1;
                                break;
                            case FRONT:
                                u =  xyz[0] / xyz[2];
                                v =  xyz[1] / xyz[2];
                                u_face = 1;
                                v_face = 1;
                                break;
                            case BACK:
                                u =  xyz[0] / xyz[2];
                                v = -xyz[1] / xyz[2];
                                u_face = 2;
                                v_face = 1;
                                break;
                            default:
                                break;
                        }
                        
                        float ew = width  / 3.0;
                        float eh = height / 2.0;
                        u *= (ew - (chroma ? 1:2)*pad)/ew;
                        v *= (eh - (chroma ? 1:2)*pad)/eh;
                        u = 0.5 * ew * (u + 1) - 0.5;
                        v = 0.5 * eh * (v + 1) - 0.5;
                        if (u < 0) u = 0;
                        if (v < 0) v = 0;
                        if (u > std::ceil(ew - 1)) u = std::ceil(ew - 1);
                        if (v > std::ceil(eh - 1)) v = std::ceil(eh - 1);
                        u += std::ceil(ew * u_face);
                        v += std::ceil(eh * v_face);
                    }
                    break;
            }
            
            if (u < 0) u = 0;
            if (v < 0) v = 0;
            if (u >= width) u = width - 1;
            if (v >= height) v = height - 1;

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
            if (ui >= width-1)
            {
                ui = width-2;
                uf = 255;
            }
            if (vi >= height-1)
            {
                vi = height-2;
                vf = 255;
            }
            arg->integerMap[2*(y*width+x)]=ui;
            arg->integerMap[2*(y*width+x)+1]=vi;
            arg->fractionalMap[2*(y*width+x)]=uf;
            arg->fractionalMap[2*(y*width+x)+1]=vf;
        }
    }
    pthread_exit(NULL);
    return NULL;
}

/**
    \fn Flat360Process_C
*/
void ADMVideoFlat360::Flat360Process_C(ADMImage *img, int w, int h, flat360 param, flat360_buffers_t * buffers)
{
    if (!img || !buffers || !buffers->imgCopy || !buffers->integerMap || !buffers->fractionalMap || !buffers->integerMapUV || !buffers->fractionalMapUV) return;
    uint8_t * line;

    if (param.algo > 1) param.algo = 1;
    unsigned int algo = param.algo;


    if (memcmp(&(buffers->prevparam), &param, sizeof(flat360)))
    {
        int totaltr = 0;

        for (int tr=0; tr<buffers->threads; tr++)
        {
            buffers->worker_thread_args[totaltr].w = w;
            buffers->worker_thread_args[totaltr].h = h;
            buffers->worker_thread_args[totaltr].chroma = false;
            buffers->worker_thread_args[totaltr].ystart = tr;
            buffers->worker_thread_args[totaltr].yincr = buffers->threads;
            buffers->worker_thread_args[totaltr].param = param;
            buffers->worker_thread_args[totaltr].integerMap = buffers->integerMap;
            buffers->worker_thread_args[totaltr].fractionalMap = buffers->fractionalMap;
            totaltr++;
        }

        for (int p=1; p<3; p++)
        {
            for (int tr=0; tr<buffers->threadsUV; tr++)
            {
                buffers->worker_thread_args[totaltr].w = w/2;
                buffers->worker_thread_args[totaltr].h = h/2;
                buffers->worker_thread_args[totaltr].chroma = true;
                buffers->worker_thread_args[totaltr].ystart = tr;
                buffers->worker_thread_args[totaltr].yincr = buffers->threadsUV;
                buffers->worker_thread_args[totaltr].param = param;
                buffers->worker_thread_args[totaltr].integerMap = buffers->integerMapUV;
                buffers->worker_thread_args[totaltr].fractionalMap = buffers->fractionalMapUV;
                totaltr++;
            }
        }

        for (int tr=0; tr<totaltr; tr++)
        {
            pthread_create( &buffers->worker_threads[tr], NULL, createMapping_worker_thread, (void*) &buffers->worker_thread_args[tr]);
        }
        // work in thread workers...
        for (int tr=0; tr<totaltr; tr++)
        {
            pthread_join( buffers->worker_threads[tr], NULL);
        }

        memcpy(&(buffers->prevparam), &param, sizeof(flat360));
    }

    uint8_t * rplanes[3];
    uint8_t * wplanes[3];
    int strides[3],imgStrides[3];

    buffers->imgCopy->duplicate(img);
    buffers->imgCopy->GetPitches(strides);
    buffers->imgCopy->GetWritePlanes(rplanes);
    img->GetWritePlanes(wplanes);
    img->GetPitches(imgStrides);
    
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
        buffers->worker_thread_args[totaltr].istride = strides[0];
        buffers->worker_thread_args[totaltr].ostride = imgStrides[0];
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
            buffers->worker_thread_args[totaltr].istride = strides[p];
            buffers->worker_thread_args[totaltr].ostride = imgStrides[p];
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
bool ADMVideoFlat360::configure()
{
    uint8_t r=0;

    r=  DIA_getFlat360(&_param, previousFilter);
    if(r) update();
    return r;
}
/**
    \fn getConfiguration
*/

const char   *ADMVideoFlat360::getConfiguration(void)
{
    static char s[256];
    const char * method=NULL;
    switch(_param.method) {
        default:
        case 0: method="Equirectangular";
            break;
        case 1: method="Equi-Angular Cubemap";
            break;
        case 2: method="Cubemap 3x2 RLUDFB";
            break;
    }
    const char * algo=NULL;
    switch(_param.algo) {
        default:
        case 0: algo="bilinear";
            break;
        case 1: algo="bicubic";
            break;
        case 2: algo="lanczos";
            break;
    }
    snprintf(s,255,"%s projection with %s interpolation", method, algo);

    return s;
}
/**
    \fn ctor
*/
ADMVideoFlat360::ADMVideoFlat360(  ADM_coreVideoFilter *in,CONFcouple *couples)  :ADM_coreVideoFilter(in,couples)
{
    if(!couples || !ADM_paramLoad(couples,flat360_param,&_param))
    {
        // Default value
        _param.method = 0;
        _param.algo = 0;
        _param.yaw = 0;
        _param.pitch = 0;
        _param.roll = 0;
        _param.fov = 90;
        _param.distortion = 0;
        _param.pad = 0;
    }
    
    Flat360CreateBuffers(info.width,info.height, &(_buffers));
    update();
}
/**
    \fn update
*/
void ADMVideoFlat360::update(void)
{
}
/**
    \fn dtor
*/
ADMVideoFlat360::~ADMVideoFlat360()
{
    Flat360DestroyBuffers(&(_buffers));
}
/**
    \fn getCoupledConf
*/
bool ADMVideoFlat360::getCoupledConf(CONFcouple **couples)
{
    return ADM_paramSave(couples, flat360_param,&_param);
}
/**
    \fn setCoupledConf
*/
void ADMVideoFlat360::setCoupledConf(CONFcouple *couples)
{
    ADM_paramLoad(couples, flat360_param, &_param);
}



/**
    \fn getNextFrame
    \brief
*/
bool ADMVideoFlat360::getNextFrame(uint32_t *fn,ADMImage *image)
{
    if(!previousFilter->getNextFrame(fn,image)) return false;

    Flat360Process_C(image,info.width,info.height,_param, &(_buffers));
 
    return 1;
}

