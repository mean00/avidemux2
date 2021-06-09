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
#include <pthread.h>

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
    upScalers   = new ADMColorScalerFull* [MOTIN_MAX_PYRAMID_LEVELS];
    downScalers = new ADMColorScalerFull* [MOTIN_MAX_PYRAMID_LEVELS];
    
    int lv,wp,hp,pw,ph;
    lv = 0;
    wp = frameW;
    hp = frameH;
    do {
        pw = wp;
        ph = hp;
        if ((wp < 128) || (hp < 128))
            break;
        pyramidA[lv] = new ADMImageDefault(wp, hp);
        pyramidB[lv] = new ADMImageDefault(wp, hp);
        pyramidWA[lv] = new ADMImageDefault(wp, hp);
        pyramidWB[lv] = new ADMImageDefault(wp, hp);
        
        wp = ((wp /4) * 2);
        hp = ((hp /4) * 2);
        upScalers[lv] = new ADMColorScalerFull(ADM_CS_BICUBIC,pw,ph,wp,hp,ADM_COLOR_YV12,ADM_COLOR_YV12);
        downScalers[lv] = new ADMColorScalerFull(ADM_CS_BICUBIC,wp,hp,pw,ph,ADM_COLOR_YV12,ADM_COLOR_YV12);
        lv += 1;
        if (lv >= MOTIN_MAX_PYRAMID_LEVELS)
            break;
    } while(1);
    
    pyramidLevels = lv;
}


motin::~motin()
{
    delete frameA;
    delete frameB;

    for (int lv=0; lv<pyramidLevels; lv++)
    {
        delete upScalers[lv];
        delete downScalers[lv];
        delete pyramidA[lv];
        delete pyramidB[lv];
        delete pyramidWA[lv];
        delete pyramidWB[lv];
    }
    
    
    delete [] upScalers;
    delete [] downScalers;
    delete [] pyramidA;
    delete [] pyramidB;
    delete [] pyramidWA;
    delete [] pyramidWB;
}


void motin::createPyramids(ADMImage * imgA, ADMImage * imgB)
{
    if (pyramidLevels < 1)
        return;
    frameA->duplicateFull(imgA);
    frameB->duplicateFull(imgB);
    pyramidA[0]->duplicateFull(imgA);
    pyramidB[0]->duplicateFull(imgB);
    
    for (int lv=0; lv<(pyramidLevels-1); lv++)
    {
        upScalers[lv]->convertImage(pyramidA[lv], pyramidA[lv+1]);
        upScalers[lv]->convertImage(pyramidB[lv], pyramidB[lv+1]);
    }
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


void *motin::worker_thread( void *ptr )
{
    worker_thread_arg * arg = (worker_thread_arg*)ptr;
    int lv = arg->lv;
    uint8_t ** plA = arg->plA;
    uint8_t ** plB = arg->plB;
    uint8_t ** plW = arg->plW;
    int * strides = arg->strides;
    uint32_t w = arg->w;
    uint32_t h = arg->h;
    int x,y,bx,by;


    for (y=0; y<h; y++)
        memset(plW[0]+y*strides[0], 128, w);
    
    w /= 2;
    h /= 2;
    
    for (y=0; y<h; y++)
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
            
            int radius = MOTIN_SEARCH_RADIUS + lv/2;
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
}
    
    
void motin::estimateMotion()
{
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
    
        pthread_t thread1, thread2;
        worker_thread_arg arg1, arg2;
        
        arg1.lv = lv;
        pyramidA[lv]->GetWritePlanes(arg1.plA);
        pyramidB[lv]->GetWritePlanes(arg1.plB);
        pyramidWA[lv]->GetWritePlanes(arg1.plW);
        pyramidA[lv]->GetPitches(arg1.strides);
        pyramidA[lv]->getWidthHeight(&arg1.w, &arg1.h);
        
        arg2.lv = lv;
        pyramidB[lv]->GetWritePlanes(arg2.plA);
        pyramidA[lv]->GetWritePlanes(arg2.plB);
        pyramidWB[lv]->GetWritePlanes(arg2.plW);
        pyramidA[lv]->GetPitches(arg2.strides);
        pyramidA[lv]->getWidthHeight(&arg2.w, &arg2.h);
        
        pthread_create( &thread1, NULL, worker_thread, (void*) &arg1);
        pthread_create( &thread2, NULL, worker_thread, (void*) &arg2);
        
        // work in thread workers...
        
        pthread_join( thread1, NULL);
        pthread_join( thread2, NULL); 
        
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
    if (alpha > 256)
        alpha = 256;
        
    int w,h,x,y,p,alphao;
    
    uint8_t * dplanes[3];
    uint8_t * wplanes[3];
    uint8_t * pplanes[3];
    int dstrides[3];
    int wstrides[3];
    int pstrides[3];

    dst->GetPitches(dstrides);
    dst->GetWritePlanes(dplanes);

    pyramidWA[0]->GetPitches(wstrides);
    pyramidWA[0]->GetWritePlanes(wplanes);
    frameA->GetPitches(pstrides);
    frameA->GetWritePlanes(pplanes);

    for (y=0; y<frameH; y++)
    {
        for (x=0; x<frameW; x++)
        {
            int mx,my;
            mx = (unsigned int)wplanes[1][x/2 + y/2*wstrides[1]];
            my = (unsigned int)wplanes[2][x/2 + y/2*wstrides[2]];
            mx -= 128;
            my -= 128;
            mx = (mx*alpha)/256;
            my = (my*alpha)/256;
            mx *= -1;
            my *= -1;
            mx += x;
            my += y;
            if ((mx < 0) || (mx >= frameW))
                continue;
            if ((my < 0) || (my >= frameH))
                continue;
            dplanes[0][x+y*dstrides[0]] = pplanes[0][mx+my*pstrides[0]];
        }
    }
    
    for (p=1; p<3; p++)
    {
        for (y=0; y<frameH/2; y++)
        {
            for (x=0; x<frameW/2; x++)
            {
                int mx,my;
                mx = (unsigned int)wplanes[1][x + y*wstrides[1]];
                my = (unsigned int)wplanes[2][x + y*wstrides[2]];
                mx -= 128;
                my -= 128;
                mx = (mx*alpha)/256;
                my = (my*alpha)/256;
                mx *= -1;
                my *= -1;
                mx /= 2;
                my /= 2;
                mx += x;
                my += y;
                if ((mx < 0) || (mx >= frameW/2))
                    continue;
                if ((my < 0) || (my >= frameH/2))
                    continue;
                dplanes[p][x+y*dstrides[p]] = pplanes[p][mx+my*pstrides[p]];
            }
        }
    }

    pyramidWB[0]->GetPitches(wstrides);
    pyramidWB[0]->GetWritePlanes(wplanes);
    frameB->GetPitches(pstrides);
    frameB->GetWritePlanes(pplanes);
    alphao = alpha;
    alpha = 256-alpha;

    for (y=0; y<frameH; y++)
    {
        for (x=0; x<frameW; x++)
        {
            int mx,my;
            mx = (unsigned int)wplanes[1][x/2 + y/2*wstrides[1]];
            my = (unsigned int)wplanes[2][x/2 + y/2*wstrides[2]];
            mx -= 128;
            my -= 128;
            mx = (mx*alpha)/256;
            my = (my*alpha)/256;
            mx *= -1;
            my *= -1;
            mx += x;
            my += y;
            if ((mx < 0) || (mx >= frameW))
                continue;
            if ((my < 0) || (my >= frameH))
                continue;
            int px = dplanes[0][x+y*dstrides[0]];
            px *= alpha;
            px += ((unsigned int)pplanes[0][mx+my*pstrides[0]])*alphao;
            dplanes[0][x+y*dstrides[0]] = px/(256);
        }
    }
    
    for (p=1; p<3; p++)
    {
        for (y=0; y<frameH/2; y++)
        {
            for (x=0; x<frameW/2; x++)
            {
                int mx,my;
                mx = (unsigned int)wplanes[1][x + y*wstrides[1]];
                my = (unsigned int)wplanes[2][x + y*wstrides[2]];
                mx -= 128;
                my -= 128;
                mx = (mx*alpha)/256;
                my = (my*alpha)/256;
                mx *= -1;
                my *= -1;
                mx /= 2;
                my /= 2;
                mx += x;
                my += y;
                if ((mx < 0) || (mx >= frameW/2))
                    continue;
                if ((my < 0) || (my >= frameH/2))
                    continue;
                int px = dplanes[p][x+y*dstrides[p]];
                px *= alpha;
                px += ((unsigned int)pplanes[p][mx+my*pstrides[p]])*alphao;
                dplanes[p][x+y*dstrides[p]] = px/(256);
            }
        }
    }

    /*pyramidWA[0]->copyPlane(pyramidWA[0], dst, (ADM_PLANE)0);

    pyramidWA[0]->copyPlane(pyramidWA[0], dst, (ADM_PLANE)1);
    pyramidWA[0]->copyPlane(pyramidWA[0], dst, (ADM_PLANE)2);*/
}


