/***************************************************************************
                          Motion interpolation
        Copyright 2021 szlldm
    Blur algorithm:
        Copyright Mario Klingemann
        Copyright Maxim Shemanarev
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "motin.h"
#include "prefs.h"
#include <cmath>

#if defined( ADM_CPU_X86) && !defined(_MSC_VER)
        #define CAN_DO_INLINE_X86_ASM
#endif

motin::motin(int width, int height)
{
    frameW = width;
    frameH = height;
    frameA = new ADMImageDefault(width, height);
    frameB = new ADMImageDefault(width, height);
    pyramidA = new ADMImage* [MOTIN_MAX_PYRAMID_LEVELS];
    pyramidB = new ADMImage* [MOTIN_MAX_PYRAMID_LEVELS];
    pyramidWA = new ADMImage* [MOTIN_MAX_PYRAMID_LEVELS];
    pyramidWB = new ADMImage* [MOTIN_MAX_PYRAMID_LEVELS];
    upScalersA   = new ADMColorScalerFull* [MOTIN_MAX_PYRAMID_LEVELS];
    upScalersB   = new ADMColorScalerFull* [MOTIN_MAX_PYRAMID_LEVELS];
    downScalers = new ADMColorScalerFull* [MOTIN_MAX_PYRAMID_LEVELS];
    
    int lv,wp,hp,pw,ph;
    lv = 0;
    wp = frameW;
    hp = frameH;
    do {
        pw = wp;
        ph = hp;
        if ((wp < 32) || (hp < 32))
            break;
        pyramidA[lv] = new ADMImageDefault(wp, hp);
        pyramidB[lv] = new ADMImageDefault(wp, hp);
        pyramidWA[lv] = new ADMImageDefault(wp, hp);
        pyramidWB[lv] = new ADMImageDefault(wp, hp);
        
        wp = ((wp /4) * 2);
        hp = ((hp /4) * 2);
        upScalersA[lv] = new ADMColorScalerFull(ADM_CS_GAUSS,pw,ph,wp,hp,ADM_COLOR_YV12,ADM_COLOR_YV12);
        upScalersB[lv] = new ADMColorScalerFull(ADM_CS_GAUSS,pw,ph,wp,hp,ADM_COLOR_YV12,ADM_COLOR_YV12);
        downScalers[lv] = new ADMColorScalerFull(ADM_CS_BILINEAR,wp,hp,pw,ph,ADM_COLOR_YV12,ADM_COLOR_YV12);
        lv += 1;
        if (lv >= MOTIN_MAX_PYRAMID_LEVELS)
            break;
    } while(1);
    
    pyramidLevels = lv;
    
    threads = ADM_cpu_num_processors();
    unithreads = threads;
    threads /= 2;
    if (threads < 1)
        threads = 1;
    if (threads > 64)
        threads = 64;
        
    me_threads1 = new pthread_t [threads];
    me_threads2 = new pthread_t [threads];
    worker_thread_args1 = new worker_thread_arg [threads];
    worker_thread_args2 = new worker_thread_arg [threads];
    
    uniw_threads = new pthread_t [unithreads];
    uniworker_thread_args = new uniworker_thread_arg [unithreads];

}


motin::~motin()
{
    delete frameA;
    delete frameB;

    for (int lv=0; lv<pyramidLevels; lv++)
    {
        delete upScalersA[lv];
        delete upScalersB[lv];
        delete downScalers[lv];
        delete pyramidA[lv];
        delete pyramidB[lv];
        delete pyramidWA[lv];
        delete pyramidWB[lv];
    }
    
    
    delete [] upScalersA;
    delete [] upScalersB;
    delete [] downScalers;
    delete [] pyramidA;
    delete [] pyramidB;
    delete [] pyramidWA;
    delete [] pyramidWB;
    delete [] me_threads1;
    delete [] me_threads2;
    delete [] worker_thread_args1;
    delete [] worker_thread_args2;
    delete [] uniw_threads;
    delete [] uniworker_thread_args;
}

void *motin::scaler_thread( void *ptr )
{
    scaler_thread_arg * arg = (scaler_thread_arg*)ptr;
    int levels = arg->levels;
    ADMColorScalerFull ** scalers = arg->scalers;
    ADMImage ** src = arg->src;
    ADMImage ** dst = arg->dst;
    
    for (int lv=0; lv<levels; lv++)
    {
        scalers[lv]->convertImage(src[lv], dst[lv]);
    }
    
    pthread_exit(NULL);

    return NULL;
}

void motin::createPyramids(ADMImage * imgA, ADMImage * imgB)
{
    if (pyramidLevels < 1)
        return;
    if ((frameW < 128) || (frameH < 128))    // can not handle small images
        return;
    
    frameA->duplicateFull(imgA);
    frameB->duplicateFull(imgB);
    pyramidA[0]->duplicateFull(imgA);
    pyramidB[0]->duplicateFull(imgB);
    
    
    long int histogramA[32], histogramB[32];
    double sum = 0.0;
    uint8_t * plA[3], *plB[3];
    uint8_t * ptrA, * ptrB;
    int strides[3];
    uint32_t w,h;
    frameA->getWidthHeight(&w,&h);
    frameA->GetPitches(strides);
    frameA->GetWritePlanes(plA);
    frameB->GetWritePlanes(plB);
    
    for (int p=0; p<3; p++)
    {
        if (p == 1)
        {
            w /= 2;
            h /= 2;
        }
        memset(histogramA, 0, sizeof(long int)*32);
        memset(histogramB, 0, sizeof(long int)*32);
        for (int y=0; y<h; y++)
        {
            ptrA = plA[p] + y*strides[p];
            ptrB = plB[p] + y*strides[p];
            for (int x=0; x<w; x++)
            {
                histogramA[(*ptrA) / 8]++;
                histogramB[(*ptrB) / 8]++;
                ptrA++;
                ptrB++;
            }
        }
        
        double tmp_sum = 0.0;
        for (int i=0; i<32; i++)
            tmp_sum += abs(histogramA[i] - histogramB[i]);
        tmp_sum /= w;
        tmp_sum /= h;
        sum += tmp_sum;
    }
    
    sum = std::sqrt(sum);
    sceneChanged = (sum > 0.5);

    if (sceneChanged)
        return;

    pthread_t scth[2];
    scaler_thread_arg args[2];
    
    args[0].levels = (pyramidLevels-1);
    args[0].scalers = upScalersA;
    args[0].src = pyramidA;
    args[0].dst = pyramidA+1;
    
    args[1].levels = (pyramidLevels-1);
    args[1].scalers = upScalersB;
    args[1].src = pyramidB;
    args[1].dst = pyramidB+1;

    pthread_create( &scth[0], NULL, scaler_thread, (void*) &args[0]);
    pthread_create( &scth[1], NULL, scaler_thread, (void*) &args[1]);
    // work in thread workers...
    pthread_join( scth[0], NULL);
    pthread_join( scth[1], NULL);

}

int motin::sad(uint8_t * p1, uint8_t * p2, int stride, int x1, int y1, int x2, int y2)
{
    volatile int tmp = 0;
    unsigned int a,b,i,j;
    uint8_t * ptrb1, * ptrb2, * ptr1, * ptr2;

    x1 -= 3;
    y1 -= 3;
    x2 -= 3;
    y2 -= 3;
    
    ptrb1 = p1 + x1 + y1*stride;
    ptrb2 = p2 + x2 + y2*stride;
    
#ifdef CAN_DO_INLINE_X86_ASM
 if(CpuCaps::hasMMX())
 {
    __asm__(
    ADM_ASM_ALIGN16
    "pxor %%mm7,%%mm7\n"
    ::);
    
    uint8_t * ptrb3, * ptrb4;
    ptrb3 = ptrb1 + stride;
    ptrb4 = ptrb2 + stride;

    for (j=0; j<4; j++)
    {
        __asm__(
        ADM_ASM_ALIGN16
        "movq (%0),%%mm0 \n"
        "movq (%1),%%mm1 \n"
        "psadbw %%mm1,%%mm0\n"
        "movq (%2),%%mm2 \n"
        "movq (%3),%%mm3 \n"
        "psadbw %%mm3,%%mm2\n"
        "paddd %%mm0,%%mm7 \n"
        "paddd %%mm2,%%mm7 \n"
        : : "r" (ptrb1) , "r" (ptrb2) , "r" (ptrb3) , "r" (ptrb4)
        );
        ptrb1 += stride*2;
        ptrb2 += stride*2;
        ptrb3 += stride*2;
        ptrb4 += stride*2;
    }
    
    __asm__(
    ADM_ASM_ALIGN16
    "movd %%mm7,(%0)\n"
    "emms \n"
    :: "r" (&tmp)
    );
 }
 else
#endif 
 {
    for (j=0; j<8; j++)
    {
            ptr1 = ptrb1;
            ptr2 = ptrb2;
            ptrb1 += stride;
            ptrb2 += stride;
        //for (i=0; i<8; i++)
        //{ unroll ->
            a = *ptr1++;
            b = *ptr2++;
            tmp += abs((int)a - (int)b);
            a = *ptr1++;
            b = *ptr2++;
            tmp += abs((int)a - (int)b);
            a = *ptr1++;
            b = *ptr2++;
            tmp += abs((int)a - (int)b);
            a = *ptr1++;
            b = *ptr2++;
            tmp += abs((int)a - (int)b);
            a = *ptr1++;
            b = *ptr2++;
            tmp += abs((int)a - (int)b);
            a = *ptr1++;
            b = *ptr2++;
            tmp += abs((int)a - (int)b);
            a = *ptr1++;
            b = *ptr2++;
            tmp += abs((int)a - (int)b);
            a = *ptr1++;
            b = *ptr2++;
            tmp += abs((int)a - (int)b);
        //}
    }
 }
    return tmp;
}


void *motin::me_worker_thread( void *ptr )
{
    worker_thread_arg * arg = (worker_thread_arg*)ptr;
    int lv = arg->lv;
    uint8_t ** plA = arg->plA;
    uint8_t ** plB = arg->plB;
    uint8_t ** plW = arg->plW;
    int * strides = arg->strides;
    uint32_t w = arg->w;
    uint32_t h = arg->h;
    uint32_t ystart = arg->ystart;
    uint32_t yincr = arg->yincr;
    int x,y,bx,by;

    int penaltyTable[MOTIN_SEARCH_RADIUS+2][MOTIN_SEARCH_RADIUS+2];

    for (y=0; y<(MOTIN_SEARCH_RADIUS+2); y++)
    {
        for (x=0; x<(MOTIN_SEARCH_RADIUS+2); x++)
        {
            penaltyTable[y][x] = std::round(std::pow(((y+.5)*(y+.5) + (x+.5)*(x+.5)), 1/3.)  * 256);  // add distance penalty <-- this looks like better than sqrt or double sqrt
        }
    }
    w /= 2;
    h /= 2;
    
    for (y=ystart; y<h; y+=yincr)    // line-by-line threading faster than partitioning
    {
        for (x=0; x<w; x++)
        {
            int initX = (unsigned int)plW[1][y*strides[1]+x];
            int initY = (unsigned int)plW[2][y*strides[2]+x];
            initX -= 128;
            initY -= 128;
            initX *= 2;
            initY *= 2;
            initX += x*2;
            initY += y*2;
            
            if ((initX < 0+3) || (initX >= w*2-1-3) || (initY < 0+3) || (initY >= h*2-1-3))
            {
                initX -= x*2;
                initY -= y*2;
                initX += 128;
                initY += 128;
                plW[1][y*strides[1]+x] = initX;
                plW[2][y*strides[2]+x] = initY;
                continue;
            }
            
            int best[2];
            best[0] = initX;
            best[1] = initY;
            int sad0 = sad(plA[0], plB[0], strides[0], x*2, y*2, initX, initY);
            
            int radius = MOTIN_SEARCH_RADIUS + ((lv > 0) ? 1:0);
            for (by=(initY-radius);by<=(initY+radius);by++)
            {
                if (by < 0+3)
                    continue;
                if (by >= (h*2-1)-3)
                    continue;
                for (bx=(initX-radius);bx<=(initX+radius);bx++)
                {
                    if (bx < 0+3)
                        continue;
                    if (bx >= (w*2-1)-3)
                        continue;
                    if ((bx == initX) && (by == initY))
                        continue;
                    
                    int sadc = sad(plA[0], plB[0], strides[0], x*2, y*2, bx, by);
                    sadc = (sadc * penaltyTable[abs(by-initY)][abs(bx-initX)]) / 256;
                    
                    if (sadc < sad0)
                    {
                        sad0 = sadc;
                        best[0] = bx;
                        best[1] = by;
                    }
                }
            }
            
            best[0] -= x*2;
            best[1] -= y*2;
            best[0] += 128;
            best[1] += 128;
            for (int bl=0; bl<2; bl++)
            {
                if (best[bl] < 16)
                    best[bl] = 16;
                if (best[bl] > 240)
                    best[bl] = 240;
            }
            plW[1][y*strides[1]+x] = best[0];
            plW[2][y*strides[2]+x] = best[1];
        }
    }

    pthread_exit(NULL);

    return NULL;
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
void motin::StackBlurLine_C(uint8_t * line, int len, int pixPitch, uint32_t * stack, unsigned int radius)
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
    uint_fast32_t sum_in_r = 0;
    uint_fast32_t sum_out_r = 0;

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
        }
        src_pix_ptr = line;
        for(i = 1; i <= radius; i++)
        {
            if(i <= lm) src_pix_ptr += pixPitch; 
            stack_pix_ptr = (uint8_t *)&stack[i + radius];
            *(uint32_t*)stack_pix_ptr = *(uint32_t*)src_pix_ptr;
            sum_in_r        += src_pix_ptr[0];
            sum_r           += src_pix_ptr[0] * (radius + 1 - i);
        }

        stack_ptr = radius;
        p = radius;
        if(p > lm) p = lm;
        src_pix_ptr = line + pixPitch*p;
        dst_pix_ptr = line;
        for(l = 0; l < len; l++)
        {
            dst_pix_ptr[0] = (sum_r * mul_sum) >> shr_sum;
            dst_pix_ptr   += pixPitch;

            sum_r -= sum_out_r;
   
            stack_start = stack_ptr + div - radius;
            if(stack_start >= div) stack_start -= div;
            stack_pix_ptr = (uint8_t *)&stack[stack_start];

            sum_out_r -= stack_pix_ptr[0];

            if(p < lm) 
                src_pix_ptr += pixPitch;
            else if(p < lm*2) 
                src_pix_ptr -= pixPitch;    // fix flickering at the B/R edges, by reflecting the image backward
            ++p;

            *(uint32_t*)stack_pix_ptr = *(uint32_t*)src_pix_ptr;

            sum_in_r += src_pix_ptr[0];
            sum_r    += sum_in_r;

            ++stack_ptr;
            if(stack_ptr >= div) stack_ptr = 0;
            stack_pix_ptr = (uint8_t *)&stack[stack_ptr];

            sum_in_r  -= stack_pix_ptr[0];
            sum_out_r += stack_pix_ptr[0];
        }
    }
}

void *motin::spf_worker_thread( void *ptr )
{
    worker_thread_arg * arg = (worker_thread_arg*)ptr;
    int lv = arg->lv;
    uint8_t ** plA = arg->plA;
    uint8_t ** plB = arg->plB;
    uint8_t ** plW = arg->plW;
    int * strides = arg->strides;
    uint32_t w = arg->w;
    uint32_t h = arg->h;
    uint32_t ystart = arg->ystart;
    uint32_t yincr = arg->yincr;
    uint32_t plane = arg->plane;
    int x,y,bx,by;

    w /= 2;
    h /= 2;
    
    // spatial filter
    #define MAX_BLUR_RADIUS (18)
    uint32_t stack[MAX_BLUR_RADIUS*2+1];
    int radius = 6*(lv+1);
    if (radius > MAX_BLUR_RADIUS) radius= MAX_BLUR_RADIUS;
    for(y = ystart; y < h; y+=yincr)
    {
        StackBlurLine_C((plW[plane] + y*strides[plane]), w, 1, stack, radius);
    }

    for(x = ystart; x < w; x+=yincr)
    {
        StackBlurLine_C((plW[plane] + x), h, strides[plane], stack, radius);
    }
    #undef MAX_BLUR_RADIUS

    pthread_exit(NULL);

    return NULL;
}

void *motin::tmf_worker_thread( void *ptr )
{
    uniworker_thread_arg * arg = (uniworker_thread_arg*)ptr;
    uint8_t ** plA = arg->wAplanes;
    uint8_t ** plB = arg->wBplanes;
    int * strides = arg->wstrides;
    uint32_t w = arg->w;
    uint32_t h = arg->h;
    uint32_t ystart = arg->ystart;
    uint32_t yincr = arg->yincr;
    uint32_t plane = arg->plane;
    int x,y,bx,by;

    w /= 2;
    h /= 2;

    // balance directional estimations

    for (y=ystart; y<h; y+=yincr)
    {
        for (x=0; x<w; x++)
        {
            int a = (unsigned int)plA[plane][y*strides[plane]+x];
            int b = (unsigned int)plB[plane][y*strides[plane]+x];
            a -= 128;
            b -= 128;
            a -= b;
            a /= 2;
            b = -1*a;
            a += 128;
            b += 128;
            plA[plane][y*strides[plane]+x] = a;
            plB[plane][y*strides[plane]+x] = b;
        }
    }

    pthread_exit(NULL);

    return NULL;
}
    
    
void motin::estimateMotion()
{
    if (sceneChanged)
        return;
    if ((frameW < 128) || (frameH < 128))    // can not handle small images
        return;

    uint8_t * planes[3];
    uint8_t * wplanes[3];
    int strides[3];
    uint32_t w,h;

    pyramidWA[pyramidLevels-1]->getWidthHeight(&w,&h);
    w /= 2;
    h /= 2;
    pyramidWA[pyramidLevels-1]->GetPitches(strides);

    pyramidWA[pyramidLevels-1]->GetWritePlanes(wplanes);
    for (int y = 0; y<h; y++)
    {
        memset(wplanes[1] + y*strides[1], 128, w);
        memset(wplanes[2] + y*strides[2], 128, w);
    }
    pyramidWB[pyramidLevels-1]->GetWritePlanes(wplanes);
    for (int y = 0; y<h; y++)
    {
        memset(wplanes[1] + y*strides[1], 128, w);
        memset(wplanes[2] + y*strides[2], 128, w);
    }


    for (int lv=pyramidLevels-1; lv>=0; lv--)
    {
        {
            uint8_t * plW[3];
            int strides[3];
            uint32_t w,h;
            
            
            pyramidWA[lv]->GetWritePlanes(plW);
            pyramidA[lv]->GetPitches(strides);
            pyramidA[lv]->getWidthHeight(&w, &h);
            for (int y=0; y<h; y++)
                memset(plW[0]+y*strides[0], 128, w);
            pyramidWB[lv]->GetWritePlanes(plW);
            for (int y=0; y<h; y++)
                memset(plW[0]+y*strides[0], 128, w);
        }

        for (int tr=0; tr<threads; tr++)
        {
            worker_thread_args1[tr].lv = lv;
            pyramidA[lv]->GetWritePlanes(worker_thread_args1[tr].plA);
            pyramidB[lv]->GetWritePlanes(worker_thread_args1[tr].plB);
            pyramidWA[lv]->GetWritePlanes(worker_thread_args1[tr].plW);
            pyramidA[lv]->GetPitches(worker_thread_args1[tr].strides);
            pyramidA[lv]->getWidthHeight(&worker_thread_args1[tr].w, &worker_thread_args1[tr].h);
            worker_thread_args1[tr].ystart = tr;
            worker_thread_args1[tr].yincr = threads;

            worker_thread_args2[tr].lv = lv;
            pyramidB[lv]->GetWritePlanes(worker_thread_args2[tr].plA);
            pyramidA[lv]->GetWritePlanes(worker_thread_args2[tr].plB);
            pyramidWB[lv]->GetWritePlanes(worker_thread_args2[tr].plW);
            pyramidA[lv]->GetPitches(worker_thread_args2[tr].strides);
            pyramidA[lv]->getWidthHeight(&worker_thread_args2[tr].w, &worker_thread_args2[tr].h);
            worker_thread_args2[tr].ystart = tr;
            worker_thread_args2[tr].yincr = threads;
        }
        for (int tr=0; tr<threads; tr++)
        {
            pthread_create( &me_threads1[tr], NULL, me_worker_thread, (void*) &worker_thread_args1[tr]);
            pthread_create( &me_threads2[tr], NULL, me_worker_thread, (void*) &worker_thread_args2[tr]);
        }
        
        // work in thread workers...
        
        for (int tr=0; tr<threads; tr++)
        {
            pthread_join( me_threads1[tr], NULL);
            pthread_join( me_threads2[tr], NULL); 
        }

        for (int dir=0; dir<2; dir++)
        {
            uint8_t * plW[3];
            int strides[3];
            uint32_t w,h;

            pyramidWA[lv]->GetPitches(strides);
            pyramidWA[lv]->getWidthHeight(&w, &h);
            if (dir==0)
                pyramidWA[lv]->GetWritePlanes(plW);
            else
                pyramidWB[lv]->GetWritePlanes(plW);

            w /= 2;
            h /= 2;

            // threat edges (motion estimate not working on the edges)
            if (lv > 0)
            {
                // top edge
                for (int p=1; p<3; p++)
                {
                    memcpy(plW[p]+0*strides[p], plW[p]+4*strides[p], w);
                    memcpy(plW[p]+1*strides[p], plW[p]+4*strides[p], w);
                    memcpy(plW[p]+2*strides[p], plW[p]+4*strides[p], w);
                    memcpy(plW[p]+3*strides[p], plW[p]+4*strides[p], w);
                }
                // bottom edge
                for (int p=1; p<3; p++)
                {
                    memcpy(plW[p]+(h-4)*strides[p], plW[p]+(h-5)*strides[p], w);
                    memcpy(plW[p]+(h-3)*strides[p], plW[p]+(h-5)*strides[p], w);
                    memcpy(plW[p]+(h-2)*strides[p], plW[p]+(h-5)*strides[p], w);
                    memcpy(plW[p]+(h-1)*strides[p], plW[p]+(h-5)*strides[p], w);
                }
                // left-right edges
                for (int p=1; p<3; p++)
                {
                    for (int y=0; y<h; y++)
                    {
                        for (int x=0;x<4;x++)
                            plW[p][x] = plW[p][4];
                        for (int x=(w-4);x<w;x++)
                            plW[p][x] = plW[p][w-5];
                    }
                }
            }
        }

        for (int dir=0; dir<2; dir++)
        {
        // spatial filters (reuse pthread and arg variables)
            for (int tr=0; tr<threads; tr++)
            {
                worker_thread_args1[tr].plane = dir+1;
                worker_thread_args2[tr].plane = dir+1;
                pthread_create( &me_threads1[tr], NULL, spf_worker_thread, (void*) &worker_thread_args1[tr]);
                pthread_create( &me_threads2[tr], NULL, spf_worker_thread, (void*) &worker_thread_args2[tr]);
            }
            
            // work in thread workers...
            
            for (int tr=0; tr<threads; tr++)
            {
                pthread_join( me_threads1[tr], NULL);
                pthread_join( me_threads2[tr], NULL); 
            }
        }
        
        // temporal filter
        for (int tr=0; tr<unithreads; tr++)
        {
            pyramidWA[lv]->GetWritePlanes(uniworker_thread_args[tr].wAplanes);
            pyramidWB[lv]->GetWritePlanes(uniworker_thread_args[tr].wBplanes);
            pyramidWA[lv]->GetPitches(uniworker_thread_args[tr].wstrides);
            pyramidWA[lv]->getWidthHeight(&uniworker_thread_args[tr].w, &uniworker_thread_args[tr].h);
            uniworker_thread_args[tr].ystart = tr;
            uniworker_thread_args[tr].yincr = unithreads;
        }
        for (int dir=0; dir<2; dir++)
        {
            for (int tr=0; tr<unithreads; tr++)
            {
                uniworker_thread_args[tr].plane = dir+1;
                pthread_create( &uniw_threads[tr], NULL, tmf_worker_thread, (void*) &uniworker_thread_args[tr]);
            }
            
            // work in thread workers...
            
            for (int tr=0; tr<unithreads; tr++)
            {
                pthread_join( uniw_threads[tr], NULL);
            }
        }
        
        for (int dir=0; dir<2; dir++)
        {
            if (lv>0)
            {
                if (dir == 0)
                {
                    downScalers[lv-1]->convertImage(pyramidWA[lv], pyramidWA[lv-1]);
                }
                else 
                {
                    downScalers[lv-1]->convertImage(pyramidWB[lv], pyramidWB[lv-1]);
                }
            }
        }
    }

}

void *motin::interp_worker_thread( void *ptr )
{
    uniworker_thread_arg * arg = (uniworker_thread_arg*)ptr;
    uint8_t ** dplanes = arg->dplanes;
    uint8_t ** wAplanes = arg->wAplanes;
    uint8_t ** wBplanes = arg->wBplanes;
    uint8_t ** pAplanes = arg->pAplanes;
    uint8_t ** pBplanes = arg->pBplanes;
    int * dstrides = arg->dstrides;
    int * wstrides = arg->wstrides;
    int * pstrides = arg->pstrides;
    uint32_t w = arg->w;
    uint32_t h = arg->h;
    uint32_t ystart = arg->ystart;
    uint32_t yincr = arg->yincr;
    uint32_t plane = arg->plane;
    int alpha = arg->alpha;


    int x,y,p,alpham,error;
    alpham = 256-alpha;

    for (y=ystart; y<h/2; y+=yincr)
    {
        for (x=0; x<w/2; x++)
        {
            int mxA,myA,mxB,myB;
            mxA = (unsigned int)wAplanes[1][x + y*wstrides[1]];
            myA = (unsigned int)wAplanes[2][x + y*wstrides[2]];
            mxB = (unsigned int)wBplanes[1][x + y*wstrides[1]];
            myB = (unsigned int)wBplanes[2][x + y*wstrides[2]];
            mxA -= 128;
            myA -= 128;
            mxB -= 128;
            myB -= 128;
            mxA = (mxA*alpha)/256;
            myA = (myA*alpha)/256;
            mxB = (mxB*alpham)/256;
            myB = (myB*alpham)/256;
            mxA *= -1;
            myA *= -1;
            mxB *= -1;
            myB *= -1;
            mxA += x*2;
            myA += y*2;
            mxB += x*2;
            myB += y*2;

            error = 0;
            if ((mxA < 0) || (mxA >= w-1) || (myA < 0) || (myA >= h-1))
                error += 1;
            if ((mxB < 0) || (mxB >= w-1) || (myB < 0) || (myB >= h-1))
                error += 2;

            switch (error)
            {
                case 0:    // both valid
                    {
                        int px, pxA, pxB;
                        uint8_t *ptrD, * ptrA, * ptrB;
                        // Luma:
                        ptrD = &dplanes[0][x*2+y*2*dstrides[0]];
                        ptrA = &pAplanes[0][mxA+myA*pstrides[0]];
                        ptrB = &pBplanes[0][mxB+myB*pstrides[0]];
                        pxA = (unsigned int)(*(ptrA));
                        pxB = (unsigned int)(*(ptrB));
                        px = (pxA*alpham + pxB*alpha)/256;
                        *(ptrD) = px;
                        pxA = (unsigned int)(*(ptrA+1));
                        pxB = (unsigned int)(*(ptrB+1));
                        px = (pxA*alpham + pxB*alpha)/256;
                        *(ptrD+1) = px;
                        ptrD += dstrides[0];
                        ptrA += pstrides[0];
                        ptrB += pstrides[0];
                        pxA = (unsigned int)(*(ptrA));
                        pxB = (unsigned int)(*(ptrB));
                        px = (pxA*alpham + pxB*alpha)/256;
                        *(ptrD) = px;
                        pxA = (unsigned int)(*(ptrA+1));
                        pxB = (unsigned int)(*(ptrB+1));
                        px = (pxA*alpham + pxB*alpha)/256;
                        *(ptrD+1) = px;
                        
                        // Chroma:
                        mxA /= 2;
                        myA /= 2;
                        mxB /= 2;
                        myB /= 2;
                        pxA = (unsigned int)pAplanes[1][mxA+myA*pstrides[1]];
                        pxB = (unsigned int)pBplanes[1][mxB+myB*pstrides[1]];
                        px = (pxA*alpham + pxB*alpha)/256;
                        dplanes[1][x+y*dstrides[1]] = px;
                        pxA = (unsigned int)pAplanes[2][mxA+myA*pstrides[2]];
                        pxB = (unsigned int)pBplanes[2][mxB+myB*pstrides[2]];
                        px = (pxA*alpham + pxB*alpha)/256;
                        dplanes[2][x+y*dstrides[2]] = px;
                    }
                    break;
                case 1:    // only B valid
                    {
                        int px, pxB;
                        uint8_t *ptrD, * ptrB;
                        // Luma:
                        ptrD = &dplanes[0][x*2+y*2*dstrides[0]];
                        ptrB = &pBplanes[0][mxB+myB*pstrides[0]];
                        pxB = (unsigned int)(*(ptrB));
                        *(ptrD) = pxB;
                        pxB = (unsigned int)(*(ptrB+1));
                        *(ptrD+1) = pxB;
                        ptrD += dstrides[0];
                        ptrB += pstrides[0];
                        pxB = (unsigned int)(*(ptrB));
                        *(ptrD) = pxB;
                        pxB = (unsigned int)(*(ptrB+1));
                        *(ptrD+1) = pxB;
                        
                        // Chroma:
                        mxB /= 2;
                        myB /= 2;
                        pxB = (unsigned int)pBplanes[1][mxB+myB*pstrides[1]];
                        dplanes[1][x+y*dstrides[1]] = pxB;
                        pxB = (unsigned int)pBplanes[2][mxB+myB*pstrides[2]];
                        dplanes[2][x+y*dstrides[2]] = pxB;

                    }
                    break;
                case 2:    // only A valid
                    {
                        int px, pxA;
                        uint8_t *ptrD, * ptrA;
                        // Luma:
                        ptrD = &dplanes[0][x*2+y*2*dstrides[0]];
                        ptrA = &pAplanes[0][mxA+myA*pstrides[0]];
                        pxA = (unsigned int)(*(ptrA));
                        *(ptrD) = pxA;
                        pxA = (unsigned int)(*(ptrA+1));
                        *(ptrD+1) = pxA;
                        ptrD += dstrides[0];
                        ptrA += pstrides[0];
                        pxA = (unsigned int)(*(ptrA));
                        *(ptrD) = pxA;
                        pxA = (unsigned int)(*(ptrA+1));
                        *(ptrD+1) = pxA;
                        
                        // Chroma:
                        mxA /= 2;
                        myA /= 2;
                        pxA = (unsigned int)pAplanes[1][mxA+myA*pstrides[1]];
                        dplanes[1][x+y*dstrides[1]] = pxA;
                        pxA = (unsigned int)pAplanes[2][mxA+myA*pstrides[2]];
                        dplanes[2][x+y*dstrides[2]] = pxA;
                    }
                    break;
                default:    // neither valid
                    // use blend
                    break;
            }

        }
    }

    pthread_exit(NULL);

    return NULL;
}

void motin::interpolate(ADMImage * dst, int alpha)
{
    if (sceneChanged)
        return;
    if ((frameW < 128) || (frameH < 128))    // can not handle small images
        return;

    if (alpha > 256)
        alpha = 256;

    for (int tr=0; tr<unithreads; tr++)
    {
        dst->GetPitches(uniworker_thread_args[tr].dstrides);
        dst->GetWritePlanes(uniworker_thread_args[tr].dplanes);
        pyramidWA[0]->GetPitches(uniworker_thread_args[tr].wstrides);
        pyramidWA[0]->GetWritePlanes(uniworker_thread_args[tr].wAplanes);
        pyramidWB[0]->GetWritePlanes(uniworker_thread_args[tr].wBplanes);
        frameA->GetPitches(uniworker_thread_args[tr].pstrides);
        frameA->GetWritePlanes(uniworker_thread_args[tr].pAplanes);
        frameB->GetWritePlanes(uniworker_thread_args[tr].pBplanes);
        frameA->getWidthHeight(&uniworker_thread_args[tr].w, &uniworker_thread_args[tr].h);
        uniworker_thread_args[tr].ystart = tr;
        uniworker_thread_args[tr].yincr = unithreads;
        uniworker_thread_args[tr].alpha = alpha;
    }

    for (int tr=0; tr<unithreads; tr++)
    {
        pthread_create( &uniw_threads[tr], NULL, interp_worker_thread, (void*) &uniworker_thread_args[tr]);
    }
    
    // work in thread workers...
    
    for (int tr=0; tr<unithreads; tr++)
    {
        pthread_join( uniw_threads[tr], NULL);
    }
}


