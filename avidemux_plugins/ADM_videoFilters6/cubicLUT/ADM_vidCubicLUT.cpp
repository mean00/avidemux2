/***************************************************************************
                          CubicLUT filter
    Algorithm:
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
#include <locale.h>
#include "ADM_default.h"
#include "ADM_coreVideoFilter.h"
#include "ADM_coreVideoFilterInternal.h"
#include "DIA_factory.h"
#include "cubicLUT.h"
#include "cubicLUT_desc.cpp"
#include "ADM_vidCubicLUT.h"
#include "ADM_image.h"
#include "ADM_imageLoader.h"
#include "ADM_colorspace.h"
#include "ADM_codec.h"
#include "fourcc.h"
#include "ADM_byteBuffer.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

extern uint8_t DIA_getCubicLUT(cubicLUT *param, ADM_coreVideoFilter *in);




// Add the hook to make it valid plugin
//DECLARE_VIDEO_FILTER(   ADMVideoCubicLUT,   // Class
DECLARE_VIDEO_FILTER_PARTIALIZABLE(   ADMVideoCubicLUT,   // Class
                                      1,0,0,              // Version
                                      ADM_UI_TYPE_BUILD,         // UI
                                      VF_COLORS,            // Category
                                      "cubicLUT",            // internal name (must be uniq!)
                                      QT_TRANSLATE_NOOP("cubicLUT","3D LUT"),            // Display name
                                      QT_TRANSLATE_NOOP("cubicLUT","Apply cubic lookup table.") // Description
                                  );


/**
    \fn CubicLUTProcess_C
*/
void ADMVideoCubicLUT::CubicLUTProcess_C(ADMImage *img, int w, int h, uint8_t * lut)
{
    if (!img || !lut) return;
    
    uint8_t * Yptr=img->GetWritePtr(PLANAR_Y);
    int Ystride=img->GetPitch(PLANAR_Y);
    uint8_t * YptrNext = Yptr+Ystride;
    uint8_t * Uptr=img->GetWritePtr(PLANAR_U);
    int Ustride=img->GetPitch(PLANAR_U);
    uint8_t * Vptr=img->GetWritePtr(PLANAR_V);
    int Vstride=img->GetPitch(PLANAR_V);
    
    uint8_t * lutBase, * lutElem;
    int U,V;

    
    for (int y=0; y<(h/2); y++)
    {
        for (int x=0; x<(w/2);x++)
        {
            lutBase = lut + (Uptr[x] + Vptr[x]*256)*256*3;
            lutElem = lutBase + Yptr[x*2]*3;
            Yptr[x*2] = lutElem[0];
            U = lutElem[1];
            V = lutElem[2];
            lutElem = lutBase + Yptr[x*2+1]*3;
            Yptr[x*2+1] = lutElem[0];
            U += lutElem[1];
            V += lutElem[2];
            lutElem = lutBase + YptrNext[x*2]*3;
            YptrNext[x*2] = lutElem[0];
            U += lutElem[1];
            V += lutElem[2];
            lutElem = lutBase + YptrNext[x*2+1]*3;
            YptrNext[x*2+1] = lutElem[0];
            U += lutElem[1];
            V += lutElem[2];
            Uptr[x] = U/4;
            Vptr[x] = V/4;
        }
        Yptr+= 2*Ystride;
        YptrNext+= 2*Ystride;
        Uptr+=Ustride;
        Vptr+=Vstride;
    }
}


/**
    \fn FileToLUT
*/
const char * ADMVideoCubicLUT::FileToLUT(const char *filename, bool hald, uint8_t * lut)
{
    if (!filename || !lut)
        return " ";
    
    float * rgblut = NULL;
    int cubicSize = 0;
    
    if (hald)
    {
        uint32_t w,h;
        if (ADM_identifyImageFile(filename,&w,&h) == ADM_PICTURE_PNG)
        {
            if (w != h)
                return QT_TRANSLATE_NOOP("cubicLUT","Width must match height");
            cubicSize = std::round(std::pow(w, 2.0/3.0));
            if (cubicSize < 2)
                return QT_TRANSLATE_NOOP("cubicLUT","Invalid resolution");
            if (w*h != cubicSize*cubicSize*cubicSize)
                return QT_TRANSLATE_NOOP("cubicLUT","Invalid resolution");
            
            int64_t fileSize = ADM_fileSize(filename);
            if (fileSize <= 0)
                return QT_TRANSLATE_NOOP("cubicLUT","Invalid file size");
            ADM_byteBuffer buffer(fileSize);
            FILE * fd = ADM_fopen(filename,"rb");
            if(fd==NULL)
                return QT_TRANSLATE_NOOP("cubicLUT","File open error");
            int64_t r = ADM_fread(buffer.at(0),fileSize,1,fd);
            ADM_fclose(fd);
            if (r!=1)
                return QT_TRANSLATE_NOOP("cubicLUT","File read error");
            
            ADMImageRef tmpImage(w,h);
            
            // Decode PNG
            decoders *dec=ADM_coreCodecGetDecoder (fourCC::get((uint8_t *)"PNG "),   w,   h, 0 , NULL,0);
            if(!dec)
                return QT_TRANSLATE_NOOP("cubicLUT","Can't find decoder");
            ADMCompressedImage bin;
            bin.data=buffer.at(0);
            bin.dataLength=fileSize;
            if(!dec->uncompress (&bin, &tmpImage))
            {
                delete dec;
                return QT_TRANSLATE_NOOP("cubicLUT","Decoding error. Only 8-bit PNG files are supported.");
            }
    
            ADM_byteBuffer compactedRGBBuffer(w*h*3);
            if (tmpImage._pixfrmt==ADM_PIXFRMT_RGB24)
            {
                for (int y=0; y<h; y++)
                    memcpy(compactedRGBBuffer.at(0)+w*3*y, tmpImage._planes[0]+tmpImage._planeStride[0]*y, w*3);
                
            }
            else
            {
                int w3 = ADM_IMAGE_ALIGN(w*3);
                ADM_byteBuffer rgbBuffer(w3*h);

                ADMColorScalerSimple converter(w,h,tmpImage._pixfrmt,ADM_PIXFRMT_RGB24);
                converter.convertImage(&tmpImage,rgbBuffer.at(0));
            
                for (int y=0; y<h; y++)
                    memcpy(compactedRGBBuffer.at(0)+w*3*y, rgbBuffer.at(0)+w3*y, w*3);
            }
            
            delete dec;
            
            rgblut = (float *)malloc(cubicSize*cubicSize*cubicSize*3*sizeof(float));
            if (rgblut == NULL)
                return QT_TRANSLATE_NOOP("cubicLUT","Memory error");

            float * fptr = rgblut;
            uint8_t * iptr = compactedRGBBuffer.at(0);

            for (int blue=0; blue<cubicSize; blue++)
            {
                for (int green=0; green<cubicSize; green++)
                {
                    for (int red=0; red<cubicSize; red++)
                    {
                        *fptr++ = (*iptr++)/255.0;
                        *fptr++ = (*iptr++)/255.0;
                        *fptr++ = (*iptr++)/255.0;
                    }

                }
            }
        }
    }
    else
    {
        int64_t fileSize = ADM_fileSize(filename);
        if (fileSize <= 0)
            return QT_TRANSLATE_NOOP("cubicLUT","Invalid file size");
        ADM_byteBuffer buffer(fileSize+3);
        FILE * fd = ADM_fopen(filename,"r");
        if(fd==NULL)
            return QT_TRANSLATE_NOOP("cubicLUT","File open error");
        int64_t r = ADM_fread(buffer.at(0),fileSize,1,fd);
        ADM_fclose(fd);
        if (r!=1)
            return QT_TRANSLATE_NOOP("cubicLUT","File read error");
        
        char * text = (char *)buffer.at(0);
        text[fileSize]=0;
        text[fileSize+1]=0;
        text[fileSize+2]=0;
        for (int i=0; i<fileSize; i++)
        {
            if (text[i]=='\r')
                text[i] = 0;
            if (text[i]=='\n')
                text[i] = 0;
        }
        
        char * line;
        int64_t ptr=0;
        bool header = true;
        float * cne = NULL;
        int ecnt = 0;
        struct lconv * lc;
        lc=localeconv();        
        char point = *(lc->decimal_point);
        while(1)
        {
            line = text + ptr;
            
            if (header)
            {
                do {
                    if (strlen(line) == 0)
                        break;
                    if (line[0] == '#')
                        break;
                    if (line[0] == ' ')
                        break;
                    if (!strncmp(line, "LUT_3D_SIZE", 11) && strlen(line) >= 13)
                    {
                        if (cubicSize)
                            return QT_TRANSLATE_NOOP("cubicLUT","Malformed header");
                        cubicSize = strtol(line + 12, NULL, 10);
                        if (cubicSize < 2)
                            return QT_TRANSLATE_NOOP("cubicLUT","Malformed header");
                        break;
                    }
                    if ((line[0] == '.') || (line[0] == '-') || (line[0] == '+') || ((line[0] >= '0') && (line[0] <= '9')))
                    {
                        if (cubicSize < 2)
                            return QT_TRANSLATE_NOOP("cubicLUT","Malformed header");
                        rgblut = (float *)malloc(cubicSize*cubicSize*cubicSize*3*sizeof(float));
                        if (rgblut == NULL)
                            return QT_TRANSLATE_NOOP("cubicLUT","Memory error");
                        cne = rgblut;
                        header = false;
                        break;
                    }

                } while(0);
            }
            
            if (!header)
            {
                if (ecnt >= cubicSize*cubicSize*cubicSize)
                    break;
                // handle locale
                for (int l=0; l<strlen(line); l++)
                {
                    if (line[l]=='.')
                        line[l] = point;
                }
                char * next;
                *cne++ = strtod(line, &next);
                if (*next!=' ')
                    break;
                next++;
                *cne++ = strtod(next, &next);
                if (*next!=' ')
                    break;
                next++;
                *cne++ = strtod(next, NULL);
                ecnt++;
            }
            
            ptr += strlen(line)+1;  // skip single \r or \n
            if (ptr >= fileSize)
                break;
            while (text[ptr] == 0)  // skip \r\n or empty lines
            {
                ptr++;
                if (ptr >= fileSize)
                    break;
            }
            if (ptr >= fileSize)
                break;
        }
        
        if (rgblut != NULL)
        {
            if (ecnt != cubicSize*cubicSize*cubicSize)
            {
                free(rgblut);
                return QT_TRANSLATE_NOOP("cubicLUT","Incomplete file");
            }
        }
    }

    if (rgblut == NULL)
        return QT_TRANSLATE_NOOP("cubicLUT","Invalid file");
    
    SparseRGBLUTtoLUT(rgblut, cubicSize, lut);
    
    free(rgblut);
    return NULL;
}


/**
    \fn rgb2lut_worker
*/
void * ADMVideoCubicLUT::rgb2lut_worker(void * ptr)
{
    worker_thread_arg * arg = (worker_thread_arg*)ptr;
    int vstart = arg->vstart;
    int vincr = arg->vincr;
    int n = arg->n;
    float * sparse = arg->sparse;
    uint8_t * lut = arg->lut;
    
    uint8_t * base;
    float Y,U,V;
    float RGB[3];
    int RGBp[3], RGBn[3];
    float diff[3];
    float RGBip[3];
    float Yo,Uo,Vo;

    for (int chromaV=vstart; chromaV<256; chromaV+=vincr)
    {
        V = (chromaV-16) / 224.0;
        if (V < 0) V = 0;
        if (V > 1) V = 1;
        V-=0.5;
        for (int chromaU=0; chromaU<256; chromaU++)
        {
            U = (chromaU-16) / 224.0;
            if (U < 0) U = 0;
            if (U > 1) U = 1;
            U-=0.5;
            for (int luma=0; luma<256; luma++)
            {
                Y = (luma-16) / 219.0;
                if (Y < 0) Y = 0;
                if (Y > 1) Y = 1;


                // BT.709
                RGB[0] = 1.0*Y +            1.5748*V;
                RGB[1] = 1.0*Y - 0.1873*U - 0.4681*V;
                RGB[2] = 1.0*Y + 1.8556*U;
                for (int i=0; i<3; i++)
                {
                    if (RGB[i] < 0) RGB[i] = 0;
                    if (RGB[i] > 1) RGB[i] = 1;
                }
                
                // trilinear interpolation:
                for (int i=0; i<3; i++)
                {
                    RGB[i] *= (n-1);
                    RGBp[i] = std::floor(RGB[i]);
                    RGBn[i] = std::ceil(RGB[i]);
                    diff[i] = (RGB[i] - RGBp[i]);
                }
                for (int i=0; i<3; i++)
                {
                    float v0 = sparse[(RGBp[0] + n*RGBp[1] + n*n*RGBp[2])*3+i]*(1-diff[0]) + sparse[(RGBn[0] + n*RGBp[1] + n*n*RGBp[2])*3+i]*diff[0];
                    float v1 = sparse[(RGBp[0] + n*RGBn[1] + n*n*RGBp[2])*3+i]*(1-diff[0]) + sparse[(RGBn[0] + n*RGBn[1] + n*n*RGBp[2])*3+i]*diff[0];
                    float v2 = sparse[(RGBp[0] + n*RGBp[1] + n*n*RGBn[2])*3+i]*(1-diff[0]) + sparse[(RGBn[0] + n*RGBp[1] + n*n*RGBn[2])*3+i]*diff[0];
                    float v3 = sparse[(RGBp[0] + n*RGBn[1] + n*n*RGBn[2])*3+i]*(1-diff[0]) + sparse[(RGBn[0] + n*RGBn[1] + n*n*RGBn[2])*3+i]*diff[0];
                    RGBip[i] = (v0*(1-diff[1])+v1*diff[1])*(1-diff[2]) + (v2*(1-diff[1])+v3*diff[1])*diff[2];
                }

                // BT.709
                Yo =  0.2126*RGBip[0] + 0.7152*RGBip[1] + 0.0722*RGBip[2];
                Uo = -0.1146*RGBip[0] - 0.3854*RGBip[1] + 0.5000*RGBip[2]     +0.5;
                Vo =  0.5000*RGBip[0] - 0.4542*RGBip[1] - 0.0458*RGBip[2]     +0.5;
                if (Yo < 0) Yo = 0;
                if (Yo > 1) Yo = 1;
                if (Uo < 0) Uo = 0;
                if (Uo > 1) Uo = 1;
                if (Vo < 0) Vo = 0;
                if (Vo > 1) Vo = 1;
                
                base = lut+((chromaU + chromaV*256)*256 + luma)*3;
                base[0] = Yo*219 + 16.49;
                base[1] = Uo*224 + 16.49;
                base[2] = Vo*224 + 16.49;
            }
        }
    }
    
    pthread_exit(NULL);
    return NULL;    
}
/**
    \fn SparseRGBLUTtoLUT
*/
void ADMVideoCubicLUT::SparseRGBLUTtoLUT(float * sparse, int n, uint8_t * lut)
{
    int threads = ADM_cpu_num_processors();
    if (threads < 1)
        threads = 1;
    if (threads > 64)
        threads = 64;
    pthread_t * pt = new pthread_t [threads];
    worker_thread_arg * warg = new worker_thread_arg [threads];
    
    for (int tr=0; tr<threads; tr++)
    {
        warg[tr].vstart = tr;
        warg[tr].vincr = threads;
        warg[tr].n = n;
        warg[tr].sparse = sparse;
        warg[tr].lut = lut;
      
        pthread_create( &pt[tr], NULL, rgb2lut_worker, (void*) &warg[tr]);
    }
    // work in thread workers...
    for (int tr=0; tr<threads; tr++)
    {
        pthread_join( pt[tr], NULL);
    }
    
    delete [] pt;
    delete [] warg;
}


/**
    \fn configure
*/
bool ADMVideoCubicLUT::configure()
{
    uint8_t r=0;

    r=  DIA_getCubicLUT(&_param, previousFilter);
    if(r) reloadLUT();
    return r;
}
/**
    \fn getConfiguration
*/

const char   *ADMVideoCubicLUT::getConfiguration(void)
{
    static char s[2560];
    snprintf(s,2559,"%s file: %s",_param.hald ? "HaldCLUT":"Cube", _param.lutfile.c_str());
    return s;
}
/**
    \fn ctor
*/
ADMVideoCubicLUT::ADMVideoCubicLUT(  ADM_coreVideoFilter *in,CONFcouple *couples)  :ADM_coreVideoFilter(in,couples)
{
    if(!couples || !ADM_paramLoad(couples,cubicLUT_param,&_param))
    {
        _param.hald = false;
    }
    _lut = (uint8_t *)malloc(256L*256L*256L*3L);
    reloadLUT();
}
/**
    \fn reloadLUT
*/
bool ADMVideoCubicLUT::reloadLUT(void)
{
        if(!_param.lutfile.size())
        {
            return false;
        }
        const char * errorMsg = FileToLUT(_param.lutfile.c_str(), _param.hald, _lut);
        if (errorMsg != NULL)
        {
            ADM_error(errorMsg);
            return false;
        }
        return true;
}
/**
    \fn dtor
*/
ADMVideoCubicLUT::~ADMVideoCubicLUT()
{
    free(_lut);
}
/**
    \fn getCoupledConf
*/
bool ADMVideoCubicLUT::getCoupledConf(CONFcouple **couples)
{
    return ADM_paramSave(couples, cubicLUT_param,&_param);
}
/**
    \fn setCoupledConf
*/
void ADMVideoCubicLUT::setCoupledConf(CONFcouple *couples)
{
    ADM_paramLoad(couples, cubicLUT_param, &_param);
}



/**
    \fn getNextFrame
    \brief
*/
bool ADMVideoCubicLUT::getNextFrame(uint32_t *fn,ADMImage *image)
{

    if(!previousFilter->getNextFrame(fn,image)) return false;

    CubicLUTProcess_C(image,info.width,info.height,_lut);
 
    return 1;
}

