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
    this->width = width;
    this->height = height;
    sceneVectorScope = scVectorScope;
    wrkVectorScope = new uint32_t [256*256];
    bufVectorScope = new uint32_t [620*600];
    imgVectorScope = new QImage((uchar *)bufVectorScope, 620, 600, 620*sizeof(uint32_t), QImage::Format_RGB32);

    sceneYUVparade = scYUVparade;
    for (int i=0; i<3; i++)
        wrkYUVparade[i] = new uint32_t [256*256];
    bufYUVparade = new uint32_t [772*258];
    imgYUVparade = new QImage((uchar *)bufYUVparade, 772, 258, 772*sizeof(uint32_t), QImage::Format_RGB32);

    sceneRGBparade = scRGBparade;
    for (int i=0; i<3; i++)
        wrkRGBparade[i] = new uint32_t [256*256];
    bufRGBparade = new uint32_t [772*258];
    imgRGBparade = new QImage((uchar *)bufRGBparade, 772, 258, 772*sizeof(uint32_t), QImage::Format_RGB32);

    sceneHistograms = scHistograms;
    for (int i=0; i<6; i++)
        wrkHistograms[i] = new uint32_t [256];
    bufHistograms = new uint32_t [772*259];
    imgHistograms = new QImage((uchar *)bufHistograms, 772, 259, 772*sizeof(uint32_t), QImage::Format_RGB32);
    
    paradeIndex = new int [width];
    for (int i=0; i<width; i++)
    {
        double fpos = i;
        fpos /= width;
        fpos *= 256.0;
        paradeIndex[i] = fpos;
        if (paradeIndex[i] > 255)
            paradeIndex[i] = 255;
    }
    paradeIndexHalf = new int [width/2];
    for (int i=0; i<width/2; i++)
    {
        double fpos = i;
        fpos *= 2.0;
        fpos /= width;
        fpos *= 256.0;
        paradeIndexHalf[i] = fpos;
        if (paradeIndexHalf[i] > 255)
            paradeIndexHalf[i] = 255;
    }

    rgbBufStride = ADM_IMAGE_ALIGN(width * 4);
    rgbBufRaw = new ADM_byteBuffer();
    rgbBufRaw->setSize(rgbBufStride * height);
    convertYuvToRgb = new ADMColorScalerFull(ADM_CS_BILINEAR,width,height,width,height,ADM_PIXFRMT_YV12,ADM_PIXFRMT_RGB32A);
}
/**
    \fn dtor
*/
flyAnalyzer::~flyAnalyzer()
{
    delete [] wrkVectorScope;
    delete [] bufVectorScope;
    delete imgVectorScope;
    for (int i=0; i<3; i++)
        delete [] wrkYUVparade[i];
    delete [] bufYUVparade;
    delete imgYUVparade;
    for (int i=0; i<3; i++)
        delete [] wrkRGBparade[i];
    delete [] bufRGBparade;
    delete imgRGBparade;
    for (int i=0; i<6; i++)
        delete [] wrkHistograms[i];
    delete [] bufHistograms;
    delete imgHistograms;
    
    delete [] paradeIndex;
    delete [] paradeIndexHalf;

    delete convertYuvToRgb;
    rgbBufRaw->clean();
    delete rgbBufRaw;
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
    
    // Make Y plane statistics
    {
        memset(wrkYUVparade[0],0,256*256*sizeof(uint32_t));
        memset(wrkHistograms[0],0,256*sizeof(uint32_t));
        uint8_t * yp=in->GetReadPtr(PLANAR_Y);
        int stride=in->GetPitch(PLANAR_Y);
        uint8_t * ptr;
        int value;
        
        for (int y=0; y<height; y++)
        {
            ptr = yp + y*stride;
            for (int x=0; x<width; x++)
            {
                value = *ptr++;
                wrkHistograms[3][value]++;
                wrkYUVparade[0][value*256 + paradeIndex[x]]++;
            }
        }
    }

    // Make U-V plane statistics
    {
        memset(wrkVectorScope,0,256*256*sizeof(uint32_t));
        memset(wrkYUVparade[1],0,256*256*sizeof(uint32_t));
        memset(wrkYUVparade[2],0,256*256*sizeof(uint32_t));
        memset(wrkHistograms[4],0,256*sizeof(uint32_t));
        memset(wrkHistograms[5],0,256*sizeof(uint32_t));
        uint8_t * up=in->GetReadPtr(PLANAR_U);
        uint8_t * vp=in->GetReadPtr(PLANAR_V);
        int ustride=in->GetPitch(PLANAR_U);
        int vstride=in->GetPitch(PLANAR_V);
        uint8_t * uptr, * vptr;
        int uvalue, vvalue;
        int width=in->GetWidth(PLANAR_U); 
        int height=in->GetHeight(PLANAR_U);
        
        for (int y=0; y<height; y++)
        {
            uptr = up + y*ustride;
            vptr = vp + y*vstride;
            for (int x=0; x<width; x++)
            {
                uvalue = *uptr++;
                vvalue = *vptr++;
                wrkHistograms[4][uvalue]+=4;    // num of chroma pixels are quarter of luma pixels
                wrkHistograms[5][vvalue]+=4;
                wrkYUVparade[1][uvalue*256 + paradeIndexHalf[x]]+=4;
                wrkYUVparade[2][vvalue*256 + paradeIndexHalf[x]]+=4;
                wrkVectorScope[vvalue*256 + uvalue]++;
            }
        }
    }
    
    // Make RGB statistics
    {
        convertYuvToRgb->convertImage(in,rgbBufRaw->at(0));
        memset(wrkRGBparade[0],0,256*256*sizeof(uint32_t));
        memset(wrkRGBparade[1],0,256*256*sizeof(uint32_t));
        memset(wrkRGBparade[2],0,256*256*sizeof(uint32_t));
        memset(wrkHistograms[0],0,256*sizeof(uint32_t));
        memset(wrkHistograms[1],0,256*sizeof(uint32_t));
        memset(wrkHistograms[2],0,256*sizeof(uint32_t));
        uint8_t * ptr;
        int rvalue, gvalue, bvalue;
 
        for (int y=0; y<height; y++)
        {
            ptr = rgbBufRaw->at(y*rgbBufStride);
            for (int x=0; x<width; x++)
            {
                rvalue = *ptr++;
                gvalue = *ptr++;
                bvalue = *ptr++;
                ptr++;
                wrkHistograms[0][rvalue]++;
                wrkHistograms[1][gvalue]++;
                wrkHistograms[2][bvalue]++;
                wrkRGBparade[0][rvalue*256 + paradeIndex[x]]++;
                wrkRGBparade[1][gvalue*256 + paradeIndex[x]]++;
                wrkRGBparade[2][bvalue*256 + paradeIndex[x]]++;
            }
        }
    }
    
    // Normalize histograms to 0 .. 124
    {
        for (int csp=0; csp<2; csp++)
        {
            uint32_t max = 0;
            for (int ch=0; ch<3; ch++)
                for (int i=0; i<256; i++)
                    if (max < wrkHistograms[csp*3+ch][i])
                        max = wrkHistograms[csp*3+ch][i];
            uint32_t norm = 2080374784ULL / max;    // 124 * 2<<24
            for (int ch=0; ch<3; ch++)
                for (int i=0; i<256; i++)
                    wrkHistograms[csp*3+ch][i] = (wrkHistograms[csp*3+ch][i]*norm)>>24;
        }
    }
    
    // Normalize parades
    {
        uint32_t norm = 2147483648ULL/(width*height);
        for (int ch=0; ch<3; ch++)
            for (int i=0; i<256; i++)
            {
                wrkYUVparade[ch][i] = (wrkYUVparade[ch][i]*norm)>>8;
                wrkRGBparade[ch][i] = (wrkRGBparade[ch][i]*norm)>>8;
            }
    }

    // Normalize vectorscope
    {
        uint32_t norm = 1073741824ULL/(width*height);
        for (int y=0; y<256; y++)
            for (int x=0; x<256; x++)
                wrkVectorScope[y*256 + x] = (wrkVectorScope[y*256 + x]*norm)>>8;
    }

    // Draw histograms
    {
        memset(bufHistograms, 0, 772*259*sizeof(uint32_t));
        uint32_t q,color;

        for (int csp=0; csp<2; csp++)
        {
            for (int ch=0; ch<3; ch++)
            {
                switch(ch+csp*3)
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
                for (int x=0; x<256; x++)
                {
                    q = wrkHistograms[csp*3+ch][x];
                    for (int y=0; y<128;y++)
                    {
                        bufHistograms[(csp*129+y+1)*772+(ch*257+1)+x] = ((((127-y)-(int)q) > 0) ? 0xFF000000 : color);
                    }
                }
            }
        }

        // add frame
        for (int x=0; x<772; x++)
        {
            bufHistograms[772*0+x] = FRAME_COLOR;
            bufHistograms[772*129+x] = FRAME_COLOR;
            bufHistograms[772*258+x] = FRAME_COLOR;
        }
        for (int y=1; y<257; y++)
        {
            bufHistograms[772*y+0] = FRAME_COLOR;
            bufHistograms[772*y+257] = FRAME_COLOR;
            bufHistograms[772*y+514] = FRAME_COLOR;
            bufHistograms[772*y+771] = FRAME_COLOR;
        }

        sceneHistograms->clear();
        sceneHistograms->addPixmap( QPixmap::fromImage(*imgHistograms));
    }
    
    // Draw YUV parade
    {
        uint32_t p,c;
        // convert to color 0xffRRGGBB
        for (int y=1; y<257; y++)
        {
            //Y
            for (int x=1; x<257; x++)
            {
                p = wrkYUVparade[0][(256-y)*256 + x-1];
                c = 0;
                if (p)
                {
                    p /= 2;
                    if (p >= 256)
                        c = 0x00FFFFFF;
                    else
                        c = (p << 16) + (p << 8) + (p << 0);
                }
                bufYUVparade[772*y+x] = 0xFF000000 | c;
            }
            //U
            for (int x=258; x<514; x++)
            {
                p = wrkYUVparade[1][(256-y)*256 + x-258];
                c = 0;
                if (p)
                {
                    if (p >= 1020)
                        c = 0x00FFFFFF;
                    else
                    if (p >= 510)
                        c = 0x00FF00FF + ((p/4) << 8);
                    else
                    if (p >= 256)
                        c = 0x000000FF + ((p/2) << 16) + ((p/4) << 8);
                    else
                        c = ((p/2) << 16) + ((p/4) << 8) + (p << 0);
                }
                bufYUVparade[772*y+x] = 0xFF000000 | c;
            }
            //V
            for (int x=515; x<771; x++)
            {
                p = wrkYUVparade[2][(256-y)*256 + x-515];
                c = 0;
                if (p)
                {
                    if (p >= 1020)
                        c = 0x00FFFFFF;
                    else
                    if (p >= 510)
                        c = 0x00FF00FF + ((p/4) << 8);
                    else
                    if (p >= 256)
                        c = 0x00FF0000 + ((p/4) << 8) + ((p/2) << 0);
                    else
                        c = (p << 16) + ((p/4) << 8) + ((p/2) << 0);
                }
                bufYUVparade[772*y+x] = 0xFF000000 | c;
            }
        }

        // add frame
        for (int x=0; x<772; x++)
        {
            bufYUVparade[772*0+x] = FRAME_COLOR;
            bufYUVparade[772*257+x] = FRAME_COLOR;
        }
        for (int y=1; y<257; y++)
        {
            bufYUVparade[772*y+0] = FRAME_COLOR;
            bufYUVparade[772*y+257] = FRAME_COLOR;
            bufYUVparade[772*y+514] = FRAME_COLOR;
            bufYUVparade[772*y+771] = FRAME_COLOR;
        }

        sceneYUVparade->clear();
        sceneYUVparade->addPixmap( QPixmap::fromImage(*imgYUVparade));
    }
    
    // Draw RGB parade
    {
         uint32_t p,c;
       // convert to color 0xffRRGGBB
        for (int y=1; y<257; y++)
        {
            //R
            for (int x=1; x<257; x++)
            {
                p = wrkRGBparade[0][(256-y)*256 + x-1];
                if (p >= 765)
                    c = 0x00FFFFFF;
                else
                if (p >= 256)
                {
                    p = (p-255)/2;
                    c = 0x00FF0000 + (p<<8) + p;
                } else
                    c = p << 16;
                bufRGBparade[772*y+x] = 0xFF000000 | c;
            }
            //G
            for (int x=258; x<514; x++)
            {
                p =wrkRGBparade[1][(256-y)*256 + x-258];
                if (p >= 765)
                    c = 0x00FFFFFF;
                else
                if (p >= 256)
                {
                    p = (p-255)/2;
                    c = 0x0000FF00 + (p<<16) + p;
                } else
                    c = p << 8;
                bufRGBparade[772*y+x] = 0xFF000000 | c;
            }
            //B
            for (int x=515; x<771; x++)
            {
                p = wrkRGBparade[2][(256-y)*256 + x-515];
                if (p >= 765)
                    c = 0x00FFFFFF;
                else
                if (p >= 256)
                {
                    p = (p-255)/2;
                    c = 0x000000FF + (p<<16) + (p<<8);
                } else
                    c = p << 0;
                bufRGBparade[772*y+x] = 0xFF000000 | c;
            }
        }

        // add frame
        for (int x=0; x<772; x++)
        {
            bufRGBparade[772*0+x] = FRAME_COLOR;
            bufRGBparade[772*257+x] = FRAME_COLOR;
        }
        for (int y=1; y<257; y++)
        {
            bufRGBparade[772*y+0] = FRAME_COLOR;
            bufRGBparade[772*y+257] = FRAME_COLOR;
            bufRGBparade[772*y+514] = FRAME_COLOR;
            bufRGBparade[772*y+771] = FRAME_COLOR;
        }

        sceneRGBparade->clear();
        sceneRGBparade->addPixmap( QPixmap::fromImage(*imgRGBparade));
    }
    
    // Draw vectorscope
    {
        uint32_t p;
        uint8_t argb[4];
        memset(bufVectorScope,0,620*600*sizeof(uint32_t));
        
        for (int y=0; y<256; y++)
            for (int x=0; x<256; x++)
                bufVectorScope[(2*y + 44)*620 + 2*x + 64] = wrkVectorScope[(255-y)*256+x];

        // interpolate histogram
        for (int y=44; y<(44+512); y+=2)
        {
            uint32_t * ptr = bufVectorScope+620*y;
            for (int x=(64-1); x<(64+512+1); x+=2)
            {
                *(ptr+x) = ( (*(ptr+x-1)) + (*(ptr+x+1)) )/2;
            }
        }
        for (int y=(44-1); y<(44+512+1); y+=2)
        {
            uint32_t * ptrm = bufVectorScope+620*(y-1);
            uint32_t * ptr  = bufVectorScope+620*y;
            uint32_t * ptrp = bufVectorScope+620*(y+1);
            for (int x=64; x<(64+512); x+=1)
            {
                *(ptr+x) = ( (*(ptrm+x)) + (*(ptrp+x)) )/2;
            }
        }

        // convert to color 0xffRRGGBB
        for (int i=0; i<620*600; i++)
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
        int i = 0;
        while (1)
        {
            if (vectorscope_scale[i][0] < 0) break;
            int x = vectorscope_scale[i][0];
            int y = vectorscope_scale[i][1];
            int k = vectorscope_scale[i][2]/2;


            memcpy(argb, bufVectorScope+620*y+x, 4);

            for (int j=0; j<3; j++)
                argb[j] = (((unsigned int)argb[j])*(255-k))>>8;

            argb[1] += k;
            argb[2] += k;

            memcpy(bufVectorScope+620*y+x, argb, 4);

            i++;
        }

        sceneVectorScope->clear();
        sceneVectorScope->addPixmap( QPixmap::fromImage(*imgVectorScope));
    }

    return 1;
}

