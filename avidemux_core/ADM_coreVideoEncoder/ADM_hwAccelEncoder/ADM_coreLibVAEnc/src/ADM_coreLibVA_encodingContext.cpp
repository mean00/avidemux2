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

#include "va/va.h"
#include "va/va_enc_h264.h"
#include "ADM_coreLibVA_buffer.h"
#include "ADM_coreLibVA_encodingContext.h"
#include "ADM_coreLibVA_h264Encoding.h"
#include "ADM_coreLibVA_hevcEncoding.h"


bool  ADM_initLibVAEncoder(void);
static bool initDone=false;

ADM_VA_GlobalH264 globalH264Caps;
ADM_VA_GlobalHEVC globalHevcCaps;
const ADM_VA_GlobalH264 *vaGetH264EncoderProfile() {return &globalH264Caps;};
const ADM_VA_GlobalHEVC *vaGetHevcEncoderProfile() {return &globalHevcCaps;};

#if 0

/**
 */
ADM_vaEncodingContext *ADM_vaEncodingContext::allocate(int codec, int alignedWidth, int alignedHeight, int frameInc,std::vector<ADM_vaSurface *>knownSurfaces)
{
    if(!initDone)
    {
        if(!init_va())
        {
            return NULL;
        }
        initDone=true;
    }
    // Allocate a new one
    ADM_vaEncodingContextH264 *r=new ADM_vaEncodingContextH264;
    if(!r->setup(alignedWidth,   alignedHeight, frameInc, knownSurfaces))
    {
        delete r;
        return NULL;
    }
    return r;
}
#endif
/* 
 */

static bool  lookupSupportedFormat(VAProfile profile)
{
    int num_entrypoints, slice_entrypoint;
    
    // query it several times, but way simpler code
    num_entrypoints = vaMaxNumEntrypoints(admLibVA::getDisplay());
    VAEntrypoint *entrypoints = (VAEntrypoint*) alloca(num_entrypoints * sizeof(VAEntrypoint));
    vaQueryConfigEntrypoints(admLibVA::getDisplay(), profile, entrypoints, &num_entrypoints);
    for (int slice_entrypoint = 0; slice_entrypoint < num_entrypoints; slice_entrypoint++) 
    {
        if (entrypoints[slice_entrypoint] == VAEntrypointEncSlice) 
        {
            return true;
        }
    }
    return false;
}
/**
 * 
 * @return 
 */
bool ADM_initLibVAEncoder(void)
{
    ADM_info("initializing VA encoder\n");
    if(lookupSupportedFormat(VAProfileHEVCMain))
    {
        ADM_info("HEVC Main is supported\n");
        globalHevcCaps.profile=VAProfileHEVCMain;
    }
    if(lookupSupportedFormat(VAProfileH264High))
    {
        ADM_info("H264 High is supported\n");
        globalH264Caps.profile=VAProfileH264High;        
    }else
    if(lookupSupportedFormat(VAProfileH264Main))
    {
        ADM_info("H264 Main is supported\n");
        globalH264Caps.profile=VAProfileH264Main;

    }
    else
    {
        ADM_warning("No support for encoding (H264 High or Main)\n");
        return false;
    }
    
    vaAttributes attributes(globalH264Caps.profile);
    if(!attributes.isSet(VAConfigAttribRTFormat,VA_RT_FORMAT_YUV420))
    {
        ADM_warning("YUV420 not supported, bailing\n");
        return false;
    }
    globalH264Caps.newAttributes.clean();
    globalH264Caps.newAttributes.add(VAConfigAttribRTFormat,VA_RT_FORMAT_YUV420);
  
    uint32_t pack=attributes.get(VAConfigAttribEncPackedHeaders);
    if(pack!=VA_ATTRIB_NOT_SUPPORTED)
    {        
        ADM_info("Support VAConfigAttribEncPackedHeaders\n");
        
        
        uint32_t new_value=VA_ENC_PACKED_HEADER_NONE;
#ifndef ADM_VA_USE_MP4_FORMAT        
#define CHECK_PACK(x) if(pack & x)         {new_value |=x;ADM_info("\t "#x" is supported\n");}
        CHECK_PACK(VA_ENC_PACKED_HEADER_SEQUENCE)
        CHECK_PACK(VA_ENC_PACKED_HEADER_PICTURE)
        CHECK_PACK(VA_ENC_PACKED_HEADER_SLICE)
        CHECK_PACK(VA_ENC_PACKED_HEADER_MISC)
#endif                
        globalH264Caps.packedHeaderCapabilities=new_value;
        globalH264Caps.newAttributes.add(VAConfigAttribEncPackedHeaders,new_value);
    }

    int ilaced=attributes.get(VAConfigAttribEncInterlaced);
    if(ilaced!=VA_ATTRIB_NOT_SUPPORTED)
    {
        globalH264Caps.newAttributes.add(VAConfigAttribEncInterlaced,VA_ENC_INTERLACED_NONE);
    }
    int h264_maxref_tmp=attributes.get(VAConfigAttribEncMaxRefFrames);
    if(h264_maxref_tmp!=VA_ATTRIB_NOT_SUPPORTED)
    {    
        globalH264Caps.h264_maxref_p0 = h264_maxref_tmp&0xffff;
        globalH264Caps.h264_maxref_p1 = h264_maxref_tmp>>16;
        ADM_info("Max ref frame is p0:%d/p1:%d\n",globalH264Caps.h264_maxref_p0,globalH264Caps.h264_maxref_p1);
    }
    ADM_info("/initializing VA encoder\n");
    return true;
}
// EOF


