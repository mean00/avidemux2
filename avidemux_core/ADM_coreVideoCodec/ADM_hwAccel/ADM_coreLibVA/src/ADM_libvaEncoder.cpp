/***************************************************************************
                          \fn ADM_VideoEncoders
                          \brief Internal handling of video encoders
                             -------------------
    
    copyright            : (C) 2018 by mean
    email                : fixounet@free.fr
 ***************************************************************************/
/* Derived from libva sample code */
/*
 * Copyright (c) 2007-2013 Intel Corporation. All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sub license, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 * 
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
 * IN NO EVENT SHALL PRECISION INSIGHT AND/OR ITS SUPPLIERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "ADM_default.h"

#include <getopt.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/mman.h>


#include "ADM_libvaEncoder.h"
#include "va/va.h"
#include "va/va_enc_h264.h"
#include "ADM_coreLibVaBuffer.h"
#include "vaDefines.h"
#include "ADM_libVaEncodingContext.h"
// setup once
static  VADisplay va_dpy;

// configuration


//--
//--

#define MIN(a, b) ((a)>(b)?(b):(a))
#define MAX(a, b) ((a)>(b)?(a):(b))


#define SRC_SURFACE_IN_ENCODING 0
#define SRC_SURFACE_IN_STORAGE  1
static  int srcsurface_status[SURFACE_NUM];
static  int encode_syncmode = 1; // not mt


    
//----
static int init_va(void);
static int render_packedsequence(void);
static int render_packedpicture(void);
static void render_packedsei(void);
static int render_picture(void);
static int render_slice(void);
static int update_ReferenceFrames(void);
static int save_codeddata(unsigned long long display_order, unsigned long long encode_order);

//---
#define CHECK_VASTATUS(va_status,func)                                  \
    if (va_status != VA_STATUS_SUCCESS) {                               \
        fprintf(stderr,"%s:%s (%d) failed,exit\n", __func__, func, __LINE__); \
        exit(1);                                                        \
    }



/**
        \fn ADM_libvaEncoder
*/
ADM_libvaEncoder::ADM_libvaEncoder(ADM_coreVideoFilter *src,bool globalHeader) : ADM_coreVideoEncoder(src)
{
    ADM_info("[LibVAEncoder] Creating.\n");
    int w,h;
    FilterInfo *info=src->getInfo();
    w=info->width;
    h=info->height;
    image=new ADMImageDefault(w,h);
    plane=(w*h*3)/2;
    vaContext=NULL;
    extraDataSize=0;
    extraData=NULL;
}
/**
 * 
 * @return 
 */
bool         ADM_libvaEncoder::setup(void)
{
    ADM_info("[LibVAEncoder] Setting up.\n");
    int w,h;
    FilterInfo *info=source->getInfo();
    w=info->width;
    h=info->height;
    std::vector<ADM_vaSurface *>xNone;
    vaContext= ADM_vaEncodingContext::allocate(0,w,h,getFrameIncrement(),xNone);
    if(!vaContext) return false;
    vaContext->generateExtraData(&(this->extraDataSize),&(this->extraData));
    return true;
}
/** 
    \fn ~ADM_libvaEncoder
*/
ADM_libvaEncoder::~ADM_libvaEncoder()
{
    ADM_info("[LibVAEncoder] Destroying.\n");
    if(vaContext)
    {
        delete vaContext;
        vaContext=NULL;
    }
    if(extraData)
    {
        delete [] extraData;
        extraData=NULL;
    }
  
}


/**
    \fn encode
*/
bool         ADM_libvaEncoder::encode (ADMBitstream * out)
{
    uint32_t fn;
    ADM_info("[LibVAEncoder] Encoding.\n");
    
    if(source->getNextFrame(&fn,image)==false)
    {
        ADM_warning("[LIBVA] Cannot get next image\n");
        return false;
    }
    bool r=vaContext->encode(image,out);
    ADM_info("Encoding frame %d, result = %d, size=%d\n",fn,r,out->len);
    return r;
}

bool         ADM_libvaEncoder::getExtraData(uint32_t *l,uint8_t **d)
{
   *l=extraDataSize;
   *d=extraData;
    return true;
}



// EOF