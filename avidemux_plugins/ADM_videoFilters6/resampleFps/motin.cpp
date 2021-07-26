/***************************************************************************
                          Motion interpolation
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
        downScalers[lv] = new ADMColorScalerFull(ADM_CS_GAUSS,wp,hp,pw,ph,ADM_COLOR_YV12,ADM_COLOR_YV12);
        lv += 1;
        if (lv >= MOTIN_MAX_PYRAMID_LEVELS)
            break;
    } while(1);
    
    pyramidLevels = lv;
    
    threads = ADM_cpu_num_processors();
    threads /= 2;
    if (threads < 1)
        threads = 1;
    if (threads > 64)
        threads = 64;
        
    me_threads1 = new pthread_t [threads];
    me_threads2 = new pthread_t [threads];
    worker_thread_args1 = new worker_thread_arg [threads];
    worker_thread_args2 = new worker_thread_arg [threads];

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

    /*for (int lv=0; lv<(pyramidLevels-1); lv++)
    {
        upScalers[lv]->convertImage(pyramidA[lv], pyramidA[lv+1]);
        upScalers[lv]->convertImage(pyramidB[lv], pyramidB[lv+1]);
    }*/
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
    int x,y,bx,by;

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
            for (y=0; y<h; y++)
            {
                for (x=0;x<4;x++)
                    plW[p][x] = plW[p][4];
                for (x=(w-4);x<w;x++)
                    plW[p][x] = plW[p][w-5];
            }
        }
    }
    
    // spatial filter
    for (y=0; y<h; y++)
    {
        for (x=0; x<w; x++)
        {
            unsigned int sumX, sumY, cnt;
            sumX = 0;
            sumY = 0;
            cnt = 0;
            for (by=y-2;by<=y+2;by++)
            {
                if (by < 0)
                    continue;
                if (by >= h)
                    continue;
                for (bx=x-2;bx<=x+2;bx++)
                {
                    if (bx < 0)
                        continue;
                    if (bx >= w)
                        continue;
                    sumX += plW[1][bx + by*strides[1]];
                    sumY += plW[2][bx + by*strides[2]];
                    cnt += 1;
                }
            }
            sumX /= cnt;
            sumY /= cnt;
            plA[1][x + y*strides[1]] = sumX;
            plA[2][x + y*strides[2]] = sumY;
        }
    }
    for (y=0; y<h; y++)
    {
        for (x=0; x<w; x++)
        {
            plW[1][x + y*strides[1]] = plA[1][x + y*strides[1]];
            plW[2][x + y*strides[2]] = plA[2][x + y*strides[2]];
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
        
        // spatial filters (reuse pthread and arg variables)
        pthread_create( &me_threads1[0], NULL, spf_worker_thread, (void*) &worker_thread_args1[0]);
        pthread_create( &me_threads2[0], NULL, spf_worker_thread, (void*) &worker_thread_args2[0]);
        // work in thread workers...
        pthread_join( me_threads1[0], NULL);
        pthread_join( me_threads2[0], NULL); 
        
        
        // filter
        {
            int x,y,bx,by;
            uint8_t * plA[3];
            uint8_t * plB[3];
            uint8_t * plW[3];
            pyramidA[lv]->GetPitches(strides);
            pyramidA[lv]->getWidthHeight(&w,&h);
            w /= 2;
            h /= 2;


            // balance directional estimations
            pyramidWA[lv]->GetWritePlanes(plA);
            pyramidWB[lv]->GetWritePlanes(plB);
            for (y=0; y<h; y++)
            {
                for (x=0; x<w; x++)
                {
                    int ax = (unsigned int)plA[1][y*strides[1]+x];
                    int ay = (unsigned int)plA[2][y*strides[2]+x];
                    int bx = (unsigned int)plB[1][y*strides[1]+x];
                    int by = (unsigned int)plB[2][y*strides[2]+x];
                    ax -= 128;
                    ay -= 128;
                    bx -= 128;
                    by -= 128;
                    ax -= bx;
                    ay -= by;
                    ax /= 2;
                    ay /= 2;
                    bx = -1*ax;
                    by = -1*ay;
                    ax += 128;
                    ay += 128;
                    bx += 128;
                    by += 128;
                    plA[1][y*strides[1]+x] = ax;
                    plA[2][y*strides[2]+x] = ay;
                    plB[1][y*strides[1]+x] = bx;
                    plB[2][y*strides[2]+x] = by;
                }
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



void motin::interpolate(ADMImage * dst, int alpha)
{
    if (sceneChanged)
        return;
    if ((frameW < 128) || (frameH < 128))    // can not handle small images
        return;

    if (alpha > 256)
        alpha = 256;
        
    int w,h,x,y,p,alpham;
    alpham = 256-alpha;
    
    uint8_t * dplanes[3];
    uint8_t * wAplanes[3];
    uint8_t * wBplanes[3];
    uint8_t * pAplanes[3];
    uint8_t * pBplanes[3];
    int dstrides[3];
    int wstrides[3];
    int pstrides[3];
    int error;

    dst->GetPitches(dstrides);
    dst->GetWritePlanes(dplanes);
    pyramidWA[0]->GetPitches(wstrides);
    pyramidWA[0]->GetWritePlanes(wAplanes);
    pyramidWB[0]->GetWritePlanes(wBplanes);
    frameA->GetPitches(pstrides);
    frameA->GetWritePlanes(pAplanes);
    frameB->GetWritePlanes(pBplanes);

    for (y=0; y<frameH/2; y++)
    {
        for (x=0; x<frameW/2; x++)
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
            if ((mxA < 0) || (mxA >= frameW-1) || (myA < 0) || (myA >= frameH-1))
                error += 1;
            if ((mxB < 0) || (mxB >= frameW-1) || (myB < 0) || (myB >= frameH-1))
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
}


