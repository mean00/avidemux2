/***************************************************************************
                          Analyzer filter 
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
#include "DIA_flyDialogQt4.h"
#include "ADM_default.h"
#include "ADM_image.h"
#include "DIA_flyAnalyzer.h"
#include "ADM_vidAnalyzer.h"
#include <QImage>
#include <QPixmap>
#include <QColor>
#include <cmath>

#include "vectorscope_scale.h"

#define FRAME_COLOR	(0xFF000000) //(0xFF7F7F7F)

/************* COMMON PART *********************/
/**
 * 
 * @param parent
 * @param width
 * @param height
 * @param in
 * @param canvas
 * @param slider
 */
flyAnalyzer::flyAnalyzer (QDialog *parent,uint32_t width,uint32_t height,ADM_coreVideoFilter *in,
                                    ADM_QCanvas *canvas, ADM_QSlider *slider, QGraphicsScene * scVectorScope,
                                    QGraphicsScene * scYUVparade, QGraphicsScene * scRGBparade, QGraphicsScene * scHistograms ) : ADM_flyDialogYuv(parent, width, height, in, canvas, slider, RESIZE_AUTO)
{
    sceneVectorScope = scVectorScope;
    bufVectorScope = (uint32_t*)malloc(620*600*sizeof(uint32_t));
    imgVectorScope = new QImage((uchar *)bufVectorScope, 620, 600, 620*sizeof(uint32_t), QImage::Format_RGB32);

    sceneYUVparade = scYUVparade;
    bufYUVparade = (uint32_t*)malloc(772*258*sizeof(uint32_t));
    imgYUVparade = new QImage((uchar *)bufYUVparade, 772, 258, 772*sizeof(uint32_t), QImage::Format_RGB32);

    sceneRGBparade = scRGBparade;
    bufRGBparade = (uint32_t*)malloc(772*258*sizeof(uint32_t));
    imgRGBparade = new QImage((uchar *)bufRGBparade, 772, 258, 772*sizeof(uint32_t), QImage::Format_RGB32);

    sceneHistograms = scHistograms;
    bufHistograms = (uint32_t*)malloc(772*259*sizeof(uint32_t));
    imgHistograms = new QImage((uchar *)bufHistograms, 772, 259, 772*sizeof(uint32_t), QImage::Format_RGB32);

    rgbWidth = width;
    rgbHeight = height;
    // scopes and histograms are limited, therefore full sized RGB may not required
    //if (rgbWidth > 512)
    //    rgbWidth = 512;
    //if (rgbHeight > 1024)
    //    rgbHeight = 1024;
    rgbBufStride = ADM_IMAGE_ALIGN(rgbWidth * 4);
    rgbBufRaw = new ADM_byteBuffer();
    if (rgbBufRaw)
        (rgbBufRaw)->setSize(rgbBufStride * rgbHeight);
    convertYuvToRgb = new ADMColorScalerFull(ADM_CS_BICUBIC,width,height,rgbWidth,rgbHeight,ADM_COLOR_YV12,ADM_COLOR_RGB32A);

}
/**
    \fn dtor
*/
flyAnalyzer::~flyAnalyzer()
{
    free(bufVectorScope);
    delete imgVectorScope;
    free(bufYUVparade);
    delete imgYUVparade;
    free(bufRGBparade);
    delete imgRGBparade;
    free(bufHistograms);
    delete imgHistograms;

    if (convertYuvToRgb) delete convertYuvToRgb;
    if (rgbBufRaw) rgbBufRaw->clean();
    if (rgbBufRaw) delete rgbBufRaw;
}
/**
    \fn update
*/
uint8_t  flyAnalyzer::update(void)
{
    return 1;
}
/**
    \fn processYuv
*/
uint8_t   flyAnalyzer::processYuv(ADMImage *in,ADMImage *out )
{
    out->duplicate(in);

    if (convertYuvToRgb && rgbBufRaw && rgbBufRaw)
    {
        convertYuvToRgb->convertImage(in,rgbBufRaw->at(0));
    }

    if (sceneVectorScope && bufVectorScope && imgVectorScope)
    {
        int width=in->GetWidth(PLANAR_U); 
        int height=in->GetHeight(PLANAR_U);

        uint8_t * u=in->GetReadPtr(PLANAR_U);
        uint8_t * v=in->GetReadPtr(PLANAR_V);
        int ustride=in->GetPitch(PLANAR_U);
        int vstride=in->GetPitch(PLANAR_V);

        int i,j,k,x,y;
        uint32_t p,max;
        uint8_t argb[4];

        memset(bufVectorScope, 0, 620*600*sizeof(uint32_t));

        // 2D U-V histogram
        for (y=0; y<height; y++)
        {
            for (x=0; x<width; x++)
            {
                bufVectorScope[620*(2*(255-v[x])+44)  +  2*u[x]+64]++;
            }
            u += ustride;
            v += vstride;
        }

        // normalize to size
        max = 1073741824ULL/(width*height);
        for (y=44; y<(44+512); y+=2)
        {
            uint32_t * ptr = bufVectorScope+620*y;
            for (x=64; x<(64+512); x+=2)
            {
                *(ptr+x) = (*(ptr+x) * max) >> 8;
            }
        }

        // interpolate histogram
        for (y=44; y<(44+512); y+=2)
        {
            uint32_t * ptr = bufVectorScope+620*y;
            for (x=(64+1); x<(64+512+1); x+=2)
            {
                *(ptr+x) = ( (*(ptr+x-1)) + (*(ptr+x+1)) )/2;
            }
        }
        for (y=(44+1); y<(44+512+1); y+=2)
        {
            uint32_t * ptrm = bufVectorScope+620*(y-1);
            uint32_t * ptr  = bufVectorScope+620*y;
            uint32_t * ptrp = bufVectorScope+620*(y+1);
            for (x=64; x<(64+512); x+=1)
            {
                *(ptr+x) = ( (*(ptrm+x)) + (*(ptrp+x)) )/2;
            }
        }

        // convert to color 0xffRRGGBB
        for (i=0; i<620*600; i++)
        {
            if (bufVectorScope[i])
            {
                p = bufVectorScope[i];
                if (p >= 765)
                {
                    bufVectorScope[i] = 0x00FFFFFF;
                } else
                if (p >= 256)
                {
                    p = (p-255)/2;
                    bufVectorScope[i] = 0x0000FF00 + (p<<16) + p;
                } else {
                    bufVectorScope[i] <<= 8;
                }
            }
            bufVectorScope[i] |= 0xFF000000;
        }

        // apply scale
        i = 0;
        while (1)
        {
            if (vectorscope_scale[i][0] < 0) break;
            x = vectorscope_scale[i][0];
            y = vectorscope_scale[i][1];
            k = vectorscope_scale[i][2]/2;


            memcpy(argb, bufVectorScope+620*y+x, 4);

            for (j=0; j<3; j++)
                argb[j] = (((unsigned int)argb[j])*(255-k))>>8;

            argb[1] += k;
            argb[2] += k;

            memcpy(bufVectorScope+620*y+x, argb, 4);

            i++;
        }

        sceneVectorScope->clear();
        sceneVectorScope->addPixmap( QPixmap::fromImage(*imgVectorScope));
    }

    if (sceneYUVparade && bufYUVparade && imgYUVparade)
    {
        int width=in->GetWidth(PLANAR_Y); 
        int height=in->GetHeight(PLANAR_Y);

        uint8_t * plane[3];
        int stride[3];
        in->GetReadPlanes(plane);
        in->GetPitches(stride);
        uint8_t * ptr;
        int i,j,x,y;
        uint32_t p,max;

        memset(bufYUVparade, 0, 772*258*sizeof(uint32_t));

        for (p=0; p<3; p++)
        {
            for (y=0; y<height; y++)
            {
                ptr = plane[p];
                for (x=0; x<width; x++)
                {
                    bufYUVparade[772*(256-ptr[x])+(p*257+1)+((x*256)/width)]++;
                }
                plane[p] += stride[p];
            }

            if (p==0)
            {
                width /= 2;
                height /= 2;
            }
        }

        // normalize to size
        max = 1073741824ULL/(width*height);
        for (y=1; y<257; y++)
        {
            for (x=1; x<771; x++)
            {
                p = bufYUVparade[772*y+x];
                bufYUVparade[772*y+x] = (p*max)>>8;
            }
        }

        // convert to color 0xffRRGGBB
        for (y=1; y<257; y++)
        {
            //Y
            for (x=1; x<257; x++)
            {
                if (bufYUVparade[772*y+x])
                {
                    p = bufYUVparade[772*y+x];
                    p /= 2;
                    if (p >= 256)
                    {
                        bufYUVparade[772*y+x] = 0x00FFFFFF;
                    } else {
                        bufYUVparade[772*y+x] = (p << 16) + (p << 8) + (p << 0);
                    }
                }
                bufYUVparade[772*y+x] |= 0xFF000000;
            }
            //U
            for (x=258; x<514; x++)
            {
                if (bufYUVparade[772*y+x])
                {
                    p = bufYUVparade[772*y+x];
                    if (p >= 1020)
                    {
                        bufYUVparade[772*y+x] = 0x00FFFFFF;
                    } else
                    if (p >= 510)
                    {
                        bufYUVparade[772*y+x] = 0x00FF00FF + ((p/4) << 8);
                    } else
                    if (p >= 256)
                    {
                        bufYUVparade[772*y+x] = 0x000000FF + ((p/2) << 16) + ((p/4) << 8);
                    } else {
                        bufYUVparade[772*y+x] = ((p/2) << 16) + ((p/4) << 8) + (p << 0);
                    }
                }
                bufYUVparade[772*y+x] |= 0xFF000000;
            }
            //V
            for (x=515; x<771; x++)
            {
                if (bufYUVparade[772*y+x])
                {
                    p = bufYUVparade[772*y+x];
                    if (p >= 1020)
                    {
                        bufYUVparade[772*y+x] = 0x00FFFFFF;
                    } else
                    if (p >= 510)
                    {
                        bufYUVparade[772*y+x] = 0x00FF00FF + ((p/4) << 8);
                    } else
                    if (p >= 256)
                    {
                        bufYUVparade[772*y+x] = 0x00FF0000 + ((p/4) << 8) + ((p/2) << 0);
                    } else {
                        bufYUVparade[772*y+x] = (p << 16) + ((p/4) << 8) + ((p/2) << 0);
                    }
                }
                bufYUVparade[772*y+x] |= 0xFF000000;
            }
        }

        // add frame
        for (x=0; x<772; x++)
        {
            bufYUVparade[772*0+x] = FRAME_COLOR;
            bufYUVparade[772*257+x] = FRAME_COLOR;
        }
        for (y=1; y<257; y++)
        {
            bufYUVparade[772*y+0] = FRAME_COLOR;
            bufYUVparade[772*y+257] = FRAME_COLOR;
            bufYUVparade[772*y+514] = FRAME_COLOR;
            bufYUVparade[772*y+771] = FRAME_COLOR;
        }

        sceneYUVparade->clear();
        sceneYUVparade->addPixmap( QPixmap::fromImage(*imgYUVparade));
    }

    if (convertYuvToRgb && rgbBufRaw && rgbBufRaw)
    if (sceneRGBparade && bufRGBparade && imgRGBparade)
    {
        int width=rgbWidth;
        int height=rgbHeight;

        uint8_t * line;
        int i,j,x,y;
        uint32_t p,max;

        memset(bufRGBparade, 0, 772*258*sizeof(uint32_t));

        for (y=0; y<height; y++)
        {
            line = rgbBufRaw->at(y*rgbBufStride);
            for (x=0; x<width; x++)
            {
                bufRGBparade[772*(256-line[x*4 + 0])+(0*257+1)+((x*256)/width)]++;
                bufRGBparade[772*(256-line[x*4 + 1])+(1*257+1)+((x*256)/width)]++;
                bufRGBparade[772*(256-line[x*4 + 2])+(2*257+1)+((x*256)/width)]++;
            }
        }

        // normalize to size
        max = 2147483648ULL/(width*height);
        for (y=1; y<257; y++)
        {
            for (x=1; x<771; x++)
            {
                p = bufRGBparade[772*y+x];
                bufRGBparade[772*y+x] = (p*max)>>8;
            }
        }

        // convert to color 0xffRRGGBB
        for (y=1; y<257; y++)
        {
            //R
            for (x=1; x<257; x++)
            {
                p = bufRGBparade[772*y+x];
                if (p >= 765)
                {
                    bufRGBparade[772*y+x] = 0x00FFFFFF;
                } else
                if (p >= 256)
                {
                    p = (p-255)/2;
                    bufRGBparade[772*y+x] = 0x00FF0000 + (p<<8) + p;
                } else {
                    bufRGBparade[772*y+x] <<= 16;
                }
                bufRGBparade[772*y+x] |= 0xFF000000;
            }
            //G
            for (x=258; x<514; x++)
            {
                p = bufRGBparade[772*y+x];
                if (p >= 765)
                {
                    bufRGBparade[772*y+x] = 0x00FFFFFF;
                } else
                if (p >= 256)
                {
                    p = (p-255)/2;
                    bufRGBparade[772*y+x] = 0x0000FF00 + (p<<16) + p;
                } else {
                    bufRGBparade[772*y+x] <<= 8;
                }
                bufRGBparade[772*y+x] |= 0xFF000000;
            }
            //B
            for (x=515; x<771; x++)
            {
                p = bufRGBparade[772*y+x];
                if (p >= 765)
                {
                    bufRGBparade[772*y+x] = 0x00FFFFFF;
                } else
                if (p >= 256)
                {
                    p = (p-255)/2;
                    bufRGBparade[772*y+x] = 0x000000FF + (p<<16) + (p<<8);
                } else {
                    bufRGBparade[772*y+x] <<= 0;
                }
                bufRGBparade[772*y+x] |= 0xFF000000;
            }
        }

        // add frame
        for (x=0; x<772; x++)
        {
            bufRGBparade[772*0+x] = FRAME_COLOR;
            bufRGBparade[772*257+x] = FRAME_COLOR;
        }
        for (y=1; y<257; y++)
        {
            bufRGBparade[772*y+0] = FRAME_COLOR;
            bufRGBparade[772*y+257] = FRAME_COLOR;
            bufRGBparade[772*y+514] = FRAME_COLOR;
            bufRGBparade[772*y+771] = FRAME_COLOR;
        }

        sceneRGBparade->clear();
        sceneRGBparade->addPixmap( QPixmap::fromImage(*imgRGBparade));
    }

    if (convertYuvToRgb && rgbBufRaw && rgbBufRaw)
    if (sceneHistograms && bufHistograms && imgHistograms)
    {
        uint8_t * line;
        uint8_t * ptr;
        int i,j,x,y;
        uint32_t p,q,max,color;

        int width=rgbWidth;
        int height=rgbHeight;

        memset(bufHistograms, 0, 772*259*sizeof(uint32_t));

        // RGB
        for (y=0; y<height; y++)
        {
            line = rgbBufRaw->at(y*rgbBufStride);
            for (x=0; x<width; x++)
            {
                bufHistograms[line[x*4 + 0]+(0*257+1)]++;
                bufHistograms[line[x*4 + 1]+(1*257+1)]++;
                bufHistograms[line[x*4 + 2]+(2*257+1)]++;
            }
        }

        width=in->GetWidth(PLANAR_Y); 
        height=in->GetHeight(PLANAR_Y);

        uint8_t * plane[3];
        int stride[3];
        in->GetReadPlanes(plane);
        in->GetPitches(stride);

        // YUV
        for (p=0; p<3; p++)
        {
            for (y=0; y<height; y++)
            {
                ptr = plane[p];
                for (x=0; x<width; x++)
                {
                    bufHistograms[129*772+ptr[x]+(p*257+1)]++;
                }
                plane[p] += stride[p];
            }

            if (p==0)
            {
                width /= 2;
                height /= 2;
            }
        }

        // normalize
        for (j=0; j<2; j++)
        {
            for (p=0; p<3; p++)
            {
                max = 0;
                for (x=0; x<256; x++)
                {
                    q = bufHistograms[j*129*772+(p*257+1)+x];
                    if (q > max) max = q;
                }
                if (max)
                {
                    max = (2080374784ULL)/max;
                    for (x=0; x<256; x++)
                    {
                        bufHistograms[j*129*772+(p*257+1)+x] *= max;
                        bufHistograms[j*129*772+(p*257+1)+x] >>= 24;
                    }
                }
            }
        }

        // draw histograms
        for (j=0; j<2; j++)
        {
            for (p=0; p<3; p++)
            {
                switch(p+j*3)
                {
                    case 0:    //R
                            color = 0xFFFF0000;
                        break;
                    case 1:    //G
                            color = 0xFF00FF00;
                        break;
                    case 2:    //B
                            color = 0xFF0000FF;
                        break;
                    case 3:    //Y
                            color = 0xFFFFFFFF;
                        break;
                    case 4:    //U
                            color = 0xFF7F3FFF;
                        break;
                    case 5:    //V
                            color = 0xFFFF3F7F;
                        break;
                }
                for (x=0; x<256; x++)
                {
                    q = bufHistograms[j*129*772+(p*257+1)+x];
                    for (y=0; y<128;y++)
                    {
                        bufHistograms[(j*129+y+1)*772+(p*257+1)+x] = ((((127-y)-(int)q) > 0) ? 0xFF000000 : color);
                    }
                }
            }
        }

        // add frame
        for (x=0; x<772; x++)
        {
            bufHistograms[772*0+x] = FRAME_COLOR;
            bufHistograms[772*129+x] = FRAME_COLOR;
            bufHistograms[772*257+x] = FRAME_COLOR;
        }
        for (y=1; y<257; y++)
        {
            bufHistograms[772*y+0] = FRAME_COLOR;
            bufHistograms[772*y+257] = FRAME_COLOR;
            bufHistograms[772*y+514] = FRAME_COLOR;
            bufHistograms[772*y+771] = FRAME_COLOR;
        }

        sceneHistograms->clear();
        sceneHistograms->addPixmap( QPixmap::fromImage(*imgHistograms));
    }

    return 1;
}

