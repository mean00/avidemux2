/***************************************************************************
                          Motion estimation
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

#include "motest.h"
#include "prefs.h"
#include <cmath>

#if defined( ADM_CPU_X86) && !defined(_MSC_VER)
        #define CAN_DO_INLINE_X86_ASM
#endif

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

motest::motest(int width, int height, int minContrast)
{
    frameW = width;
    frameH = height;
    frameA = new ADMImageDefault(width, height);
    frameB = new ADMImageDefault(width, height);
    pyramidA = new ADMImage* [MOTEST_MAX_PYRAMID_LEVELS];
    pyramidB = new ADMImage* [MOTEST_MAX_PYRAMID_LEVELS];
    pyramidWA = new ADMImage* [MOTEST_MAX_PYRAMID_LEVELS];
    upScalers   = new ADMColorScalerFull* [MOTEST_MAX_PYRAMID_LEVELS];
    downScalers = new ADMColorScalerFull* [MOTEST_MAX_PYRAMID_LEVELS];
    
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
        
        wp = ((wp /4) * 2);
        hp = ((hp /4) * 2);
        upScalers[lv] = new ADMColorScalerFull(ADM_CS_LANCZOS,pw,ph,wp,hp,ADM_PIXFRMT_YV12,ADM_PIXFRMT_YV12);
        downScalers[lv] = new ADMColorScalerFull(ADM_CS_LANCZOS,wp,hp,pw,ph,ADM_PIXFRMT_YV12,ADM_PIXFRMT_YV12);
        lv += 1;
        if (lv >= MOTEST_MAX_PYRAMID_LEVELS)
            break;
    } while(1);
    
    pyramidLevels = lv;
    
    threads = ADM_cpu_num_processors();
    if (threads < 1)
        threads = 1;
    if (threads > 64)
        threads = 64;
        
    me_threads1 = new pthread_t [threads];
    me_threads2 = new pthread_t [threads];
    worker_thread_args1 = new worker_thread_arg [threads];
    worker_thread_args2 = new worker_thread_arg [threads];
    
    validPrevFrame = 0;
    contrastThreshold = minContrast;
    
    motionMap[0] = new int [(width/2)*(height/2)];
    motionMap[1] = new int [(width/2)*(height/2)];
    contrastMap = new int [(width/2)*(height/2)];
    angleMap = new double [(width/2)*(height/2)];
    for (int j=0; j<(height/2); j++)
    {
        for (int i=0; i<(width/2); i++)
        {
            angleMap[j*(width/2) + i] = std::atan2(j-(height/4), i-(width/4));
        }
    }

}


motest::~motest()
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
    }
    
    
    delete [] upScalers;
    delete [] downScalers;
    delete [] pyramidA;
    delete [] pyramidB;
    delete [] pyramidWA;
    delete [] me_threads1;
    delete [] me_threads2;
    delete [] worker_thread_args1;
    delete [] worker_thread_args2;
    delete [] motionMap[0];
    delete [] motionMap[1];
    delete [] contrastMap;
    delete [] angleMap;
}

void motest::addNextImage(ADMImage * img)
{
    if (img == NULL)
    {
        validPrevFrame = 0;
        return;
    }
    if (pyramidLevels < 1)
        return;
    if ((frameW < 128) || (frameH < 128))    // can not handle small images
        return;
    if (validPrevFrame < 2)
        validPrevFrame++;

    ADMImage * swap;
    swap = frameB;
    frameB = frameA;
    frameA = swap;
    frameB->duplicateFull(img);
    
    ADMImage ** swapy;
    swapy = pyramidB;
    pyramidB = pyramidA;
    pyramidA = swapy;
    pyramidB[0]->duplicateFull(img);

    for (int lv=0; lv<(pyramidLevels-1); lv++)
    {
        upScalers[lv]->convertImage(pyramidB[lv], pyramidB[lv+1]);
    }
}

int motest::sad(uint8_t * p1, uint8_t * p2, int stride, int x1, int y1, int x2, int y2)
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


void *motest::me_worker_thread( void *ptr )
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
    unsigned int speedup = arg->speedup;
    int * motionMap[2];
    motionMap[0] = arg->motionMap[0];
    motionMap[1] = arg->motionMap[1];
    int * contrastMap = arg->contrastMap;
    int x,y,bx,by;

    int penaltyTable[MOTEST_SEARCH_RADIUS+2][MOTEST_SEARCH_RADIUS+2];

    for (y=0; y<(MOTEST_SEARCH_RADIUS+2); y++)
    {
        for (x=0; x<(MOTEST_SEARCH_RADIUS+2); x++)
        {
            penaltyTable[y][x] = std::round(std::pow((y*y + x*x), 1/3.)  * 256);  // add distance penalty <-- this looks like better than sqrt or double sqrt
        }
    }
    w /= 2;
    h /= 2;
    
    for (y=ystart; y<h; y+=yincr)    // line-by-line threading faster than partitioning
    {
        if ((lv==0)&&(speedup>0))
        {
            if (y%8)
                continue;
        }
        if ((lv==1)&&(speedup>0))
        {
            if (y%4)
                continue;
        }
        for (x=0; x<w; x++)
        {
            if ((lv==0)&&(speedup>0))
            {
                if (x%8)
                    continue;
            }
            if ((lv==1)&&(speedup>0))
            {
                if (x%4)
                    continue;
            }
            
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
            
            if (lv==0)
            {
                int pix, pmin=255,pmax=0;
                bool fail=false;
                for (by=(y*2-3);by<=(y*2+4);by++)
                {
                    if ((by < 0+3) || (by >= (h*2-1)-3))
                    {
                        fail = true;
                        break;
                    }
                    for (bx=(x*2-3);bx<=(x*2+4);bx++)
                    {
                        if ((bx < 0+3) || (bx >= (w*2-1)-3))
                        {
                            fail = true;
                            break;
                        }
                        pix = plA[0][by*strides[0]+bx];
                        if (pmin > pix)
                            pmin = pix;
                        if (pmax < pix)
                            pmax = pix;
                    }
                    if (fail)
                        break;
                }
                
                if (fail)
                    continue;

                contrastMap[y*w+x] = (pmax - pmin);
            }
            
            int best[2];
            best[0] = initX;
            best[1] = initY;
            int sad0 = sad(plA[0], plB[0], strides[0], x*2, y*2, initX, initY);
            
            int radius = MOTEST_SEARCH_RADIUS + ((lv > 0) ? 1:0);
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
            if (lv==0)
            {
                motionMap[0][y*w+x] = best[0];
                motionMap[1][y*w+x] = best[1];
            }
            else
            {
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
                if ((lv==1)&&(speedup>0))
                {
                    for (by=(y-2);by<=(y+2);by++)
                    {
                        if (by < 0)
                            continue;
                        if (by >= h)
                            continue;
                        for (bx=(x-2);bx<=(x+2);bx++)
                        {
                            if (bx < 0)
                                continue;
                            if (bx >= w)
                                continue;
                            if ((bx == x) && (by == y))
                                continue;
                            plW[1][by*strides[1]+bx] = best[0];
                            plW[2][by*strides[2]+bx] = best[1];
                        }
                    }
                }
            }
        }
    }

    pthread_exit(NULL);
    return NULL;
}

void *motest::spf_worker_thread( void *ptr )
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

        // spatial filter
        for (y=0; y<h; y++)
        {
            for (x=0; x<w; x++)
            {
                unsigned int sumX, sumY, cnt;
                sumX = 0;
                sumY = 0;
                cnt = 0;
                for (by=y-1;by<=y+1;by++)
                {
                    if (by < 0)
                        continue;
                    if (by >= h)
                        continue;
                    for (bx=x-1;bx<=x+1;bx++)
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
    }

    pthread_exit(NULL);
    return NULL;
}

    
    
void motest::estimateMotion(unsigned int speed)
{
    if (validPrevFrame < 2)
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

    memset(contrastMap, 0, sizeof(int)*(frameW/2)*(frameH/2));


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
            worker_thread_args1[tr].speedup = speed;
            worker_thread_args1[tr].contrastMap = contrastMap;
            worker_thread_args1[tr].motionMap[0] = motionMap[0];
            worker_thread_args1[tr].motionMap[1] = motionMap[1];
        }
        for (int tr=0; tr<threads; tr++)
        {
            pthread_create( &me_threads1[tr], NULL, me_worker_thread, (void*) &worker_thread_args1[tr]);
        }
        
        // work in thread workers...
        
        for (int tr=0; tr<threads; tr++)
        {
            pthread_join( me_threads1[tr], NULL);
        }
        
        // spatial filters (reuse pthread and arg variables)
        pthread_create( &me_threads1[0], NULL, spf_worker_thread, (void*) &worker_thread_args1[0]);
        // work in thread workers...
        pthread_join( me_threads1[0], NULL);

        if (lv>0)
        {
            downScalers[lv-1]->convertImage(pyramidWA[lv], pyramidWA[lv-1]);
        }
    }
}

void motest::getMotionParameters(double * global, double * rotation)
{
    if ((!global) || (!rotation))
        return;
    memset(global, 0, 2*sizeof(double));
    *rotation = 0.0;
    
    if (validPrevFrame < 2)
        return;
    if ((frameW < 128) || (frameH < 128))    // can not handle small images
        return;
        
    int x,y,w,h;
    w = frameW/2;
    h = frameH/2;
    
    int count;
    double avgx = 0.0;
    double avgy = 0.0;
    count = 0;
    for(y=0; y<h; y++)
    {
        for(x=0; x<w; x++)
        {
            if (contrastMap[y*w+x] >= contrastThreshold)
            {
                avgx += motionMap[0][x + y*w];
                avgy += motionMap[1][x + y*w];
                count++;
            }
        }
    }
    if (count == 0)
        return;
    global[0] = avgx / count;
    global[1] = avgy / count;

    double resx,resy,angle,avgangle;
    int mx,my;
    avgangle = 0.0;
    count = 0;
    for(y=0; y<h; y++)
    {
        for(x=0; x<w; x++)
        {
            if ((y > (h/4)) && (y < (h-h/4)))
            {
                if (x == w/4)
                    x = w-w/4;    // skip center
            }
            if (contrastMap[y*w+x] >= contrastThreshold)
            {
                
                resx = motionMap[0][x + y*w];
                resy = motionMap[1][x + y*w];
                resx -= global[0];
                resy -= global[1];
                mx = std::round(resx/2);
                my = std::round(resy/2);
                mx += x;
                my += y;
                if ((mx < 0) || (mx >= w) || (my < 0) || (my >= h))
                    continue;
                angle = angleMap[my*w+mx] - angleMap[y*w+x];
                if (angle > M_PI)
                    angle -= 2.0*M_PI;
                else
                if (angle < -1.0*M_PI)
                    angle += 2.0*M_PI;
                if ((angle > -1.0*(M_PI/8)) && (angle < (M_PI/8)))    // limit to between +- 22.5 degree
                {
                    avgangle += angle;
                    count++;
                }
            }
        }
    }
    if (count == 0)
        return;
    *rotation = avgangle / count;
}


