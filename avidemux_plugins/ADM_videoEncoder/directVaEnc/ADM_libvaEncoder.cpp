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
#include "ADM_coreLibVA_buffer.h"
#include "vaDefines.h"
#include "ADM_coreLibVA_encodingContext.h"
#include "ADM_libVaEncodingContextH264.h"

/**
        \fn ADM_libvaEncoder
*/
ADM_libvaEncoder::ADM_libvaEncoder(ADM_coreVideoFilter *src,bool globalHeader) : ADM_coreVideoEncoder(src)
{
    ADM_info("[LibVAEncoder] Creating, globalHeader=%d.\n",globalHeader);
    image=new ADMImageDefault(getWidth(),getHeight());
    vaContext=NULL;
    extraDataSize=0;
    extraData=NULL;
    this->globalHeader=globalHeader;
}
/**
 * 
 * @return 
 */
bool         ADM_libvaEncoder::setup(void)
{
    ADM_info("[LibVAEncoder] Setting up.\n");
    int w=getWidth(),h=getHeight();
    std::vector<ADM_vaSurface *>xNone;
#if 0
    vaContext= ADM_vaEncodingContext::allocate(0,w,h,getFrameIncrement(),xNone);
    if(!vaContext) 
        return false;
#else
     // Allocate a new one
    ADM_vaEncodingContextH264Base *ctx;
    ctx=new ADM_vaEncodingContextH264AnnexB(globalHeader);
    if(!ctx->setup(w,   h, getFrameIncrement(), xNone))
    {
        delete ctx;
        ctx=NULL;        
        return false;
    }
    vaContext=ctx;
#endif
    
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
    aprintf("[LibVAEncoder] Encoding.\n");
    
    if(source->getNextFrame(&fn,image)==false)
    {
        ADM_warning("[LIBVA] Cannot get next image\n");
        return false;
    }
    bool r=vaContext->encode(image,out);
    aprintf("Encoding frame %d, result = %d, size=%d\n",fn,r,out->len);
    return r;
}

bool         ADM_libvaEncoder::getExtraData(uint32_t *l,uint8_t **d)
{
   *l=extraDataSize;
   *d=extraData;
    return true;
}



// EOF
