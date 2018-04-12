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
#include "ADM_coreLibVA_buffer.h"

#define CHECK_ERROR_BOOL(x) {xError=x;if(xError) {ADM_warning(#x "%d =<%s>\n",xError,vaErrorStr(xError));return false;}}
#define CHECK_ERROR(x)      {xError=x;if(xError) {ADM_warning(#x "%d =<%s>\n",xError,vaErrorStr(xError));}}

static VAStatus xError; // this is not thread safe !

ADM_vaEncodingBuffers::ADM_vaEncodingBuffers()
{
    _bufferId=VA_INVALID;
}
/**
 * 
 * @param ctx
 * @param size
 * @return 
 */
bool        ADM_vaEncodingBuffers::setup(VAContextID ctx, int size)
{
        CHECK_ERROR_BOOL( vaCreateBuffer(admLibVA::getDisplay(),ctx,VAEncCodedBufferType,size, 1, NULL, &_bufferId));
        return true;
}
/**
 */
ADM_vaEncodingBuffers::~ADM_vaEncodingBuffers()
{
    if(_bufferId!=VA_INVALID)
    {
           CHECK_ERROR(vaDestroyBuffer(admLibVA::getDisplay(),_bufferId));
           _bufferId=VA_INVALID;
    }
}
/**
 */
ADM_vaEncodingBuffers *ADM_vaEncodingBuffers::allocate(VAContextID context, int size)
{
    ADM_vaEncodingBuffers *b=new ADM_vaEncodingBuffers;
    if(!b->setup(context,size))
    {
        ADM_warning("VaEncoder: Buffer creation failed\n");
        delete b;
        return NULL;
    }
    return b;
}
/**
 * 
 * @param to
 * @param sizeMax
 * @return 
 */
int ADM_vaEncodingBuffers::read(uint8_t *data, int sizeMax)
{
    VACodedBufferSegment *buf_list = NULL;    
    xError= vaMapBuffer(admLibVA::getDisplay(),_bufferId,(void **)(&buf_list));
    CHECK_ERROR(xError)
    if(xError)
    {
        return -1;
    }
    int len=0;
    while (buf_list != NULL) 
    {
        int sz=buf_list->size;
        if(sz+len>sizeMax)
        {
            printf("VA buffer readback buffer size exceeded !");
            ADM_assert(0);
        }
        memcpy(data,buf_list->buf,sz);
        data+=sz;
        len+=sz;
        buf_list = (VACodedBufferSegment *) buf_list->next;
    }
    vaUnmapBuffer(admLibVA::getDisplay(),_bufferId);
    return len;
}

// EOF
