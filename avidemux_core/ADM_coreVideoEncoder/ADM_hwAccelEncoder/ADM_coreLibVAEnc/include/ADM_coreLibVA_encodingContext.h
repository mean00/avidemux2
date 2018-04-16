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

#include "ADM_h264_tag.h"
#pragma once


#define CHECK_VA_STATUS_BOOL(x)     {VAStatus status=x; if(status!=VA_STATUS_SUCCESS) \
            { ADM_warning("%s failed at line %d function %s, err code=%d\n",#x,__LINE__,__func__,(int)status);return false;}}
#include <vector>
class ADMImage;
class ADM_vaSurface;
class ADMBitstream;

/**
 * 
 * @param codec
 * @param alignedWidth
 * @param alignedHeight
 * @param knownSurfaces
 * @return 
 */
class ADM_vaEncodingContext
{
public:
                ADM_vaEncodingContext() {}
    virtual      ~ADM_vaEncodingContext() {}
    virtual bool encode(ADMImage *in, ADMBitstream *out)=0;
    virtual bool generateExtraData(int *size, uint8_t **data)=0;
    //static       ADM_vaEncodingContext *allocate(int codec, int width, int height, int frameInc,std::vector<ADM_vaSurface *>knownSurfaces);    
};


/**
 * 
 * @param profile
 */
class vaAttributes
{
public:
        vaAttributes(VAProfile profile)
        {            
            for (int i = 0; i < VAConfigAttribTypeMax; i++)
                attrib[i].type = (VAConfigAttribType)i;
            ADM_assert(VA_STATUS_SUCCESS==vaGetConfigAttributes(admLibVA::getDisplay(), profile, VAEntrypointEncSlice,  &attrib[0], VAConfigAttribTypeMax));
        }
        bool isSet(int attribute, int mask)
        {
            return attrib[attribute].value & mask;
        }
        uint32_t get(VAConfigAttribType key)
        {
            return attrib[key].value;
        }

protected:
        VAConfigAttrib attrib[VAConfigAttribTypeMax];
      
};
/**
 * 
 */
class vaSetAttributes
{
    
public:
    vaSetAttributes()
    {
        xindex=0;
    }
    void add(VAConfigAttribType key, int value)
    {
        attrib[xindex].type=key;
        attrib[xindex].value=value;
        xindex++;
    }
    void clean(void)
    {
      xindex=0;
    }
    int count() const {return xindex;};
    const VAConfigAttrib *getPointer() const 
    {
      return &attrib[0];
    }

protected:
        VAConfigAttrib attrib[VAConfigAttribTypeMax];    
        int xindex;
};
