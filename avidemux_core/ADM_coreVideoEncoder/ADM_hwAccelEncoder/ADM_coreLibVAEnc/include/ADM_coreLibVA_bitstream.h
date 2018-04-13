/***************************************************************************
                          \fn     libvaEnc_plugin
                          \brief  Plugin to use libva hw encoder (intel mostly)
                             -------------------

    copyright            : (C) 2018 by mean
    email                : fixounet@free.fr
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
 /***************************************************************************/
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
#pragma once
/**
 * 
 */
class vaBitstream
{
public:
                vaBitstream();
                ~vaBitstream();
        void    stop();
        void    put_ui(unsigned int val,int size_in_bits);
        void    put_ue(unsigned int val);
        void    put_se(signed int val);
        void    byteAlign(signed int bit);
        void    rbspTrailingBits();
        void    startCodePrefix();
        void    nalHeader(int nal_ref_idc, int nal_unit_type);
        int     lengthInBits() {return bit_offset;};
        int     lengthInBytes() { int l=bit_offset;return (l+7)>>3;};
        void    add1BitIfNotaligned()
        {
                if ( bit_offset & 0x7) 
                {
                    put_ui(1, 1);
                }
        }
        uint8_t *getPointer() {return  (uint8_t *)buffer;}
protected:
    unsigned int *buffer;
    int          bit_offset;
    int          max_size_in_dword;
};
