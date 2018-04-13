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
#include "ADM_default.h"
#include "va/va.h"
#include "ADM_coreLibVA_bitstream.h"
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
#define BITSTREAM_ALLOCATE_STEPPING     4096


static unsigned int 
va_swap32(unsigned int val)
{
    unsigned char *pval = (unsigned char *)&val;

    return ((pval[0] << 24)     |
            (pval[1] << 16)     |
            (pval[2] << 8)      |
            (pval[3] << 0));
}

vaBitstream::vaBitstream()
{
    max_size_in_dword = BITSTREAM_ALLOCATE_STEPPING;
    buffer = (unsigned int *)calloc(max_size_in_dword * sizeof(int), 1);
    bit_offset = 0; 
}
vaBitstream::~vaBitstream()
{
    free(buffer);
    buffer=NULL;
}
void vaBitstream::stop()
{
    int pos = (bit_offset >> 5);
    int xbit_offset = (bit_offset & 0x1f);
    int bit_left = 32 - xbit_offset;

    if (xbit_offset) 
    {
        buffer[pos] = va_swap32((buffer[pos] << bit_left));
    }
}

void vaBitstream::put_ui(unsigned int val, int size_in_bits)
{
    int pos = (bit_offset >> 5);
    int xbit_offset = (bit_offset & 0x1f);
    int bit_left = 32 - xbit_offset;

    if (!size_in_bits)
        return;

    bit_offset += size_in_bits;

    if (bit_left > size_in_bits) {
        buffer[pos] = (buffer[pos] << size_in_bits | val);
    } else {
        size_in_bits -= bit_left;
        buffer[pos] = (buffer[pos] << bit_left) | (val >> size_in_bits);
        buffer[pos] = va_swap32(buffer[pos]);

        if (pos + 1 == max_size_in_dword) {
            max_size_in_dword += BITSTREAM_ALLOCATE_STEPPING;
            buffer = (unsigned int *)realloc(buffer, max_size_in_dword * sizeof(unsigned int));
        }

        buffer[pos + 1] = val;
    }
}

void vaBitstream::put_ue(unsigned int val)
{
    int size_in_bits = 0;
    int tmp_val = ++val;

    while (tmp_val) {
        tmp_val >>= 1;
        size_in_bits++;
    }

    put_ui( 0, size_in_bits - 1); // leading zero
    put_ui( val, size_in_bits);
}

void vaBitstream::put_se(int val)
{
    unsigned int new_val;

    if (val <= 0)
        new_val = -2 * val;
    else
        new_val = 2 * val - 1;

    put_ue(new_val);
}

void vaBitstream::byteAlign(int bit)
{
    int xbit_offset = (bit_offset & 0x7);
    int bit_left = 8 - xbit_offset;
    int new_val;

    if (!xbit_offset)
        return;

    assert(bit == 0 || bit == 1);

    if (bit)
        new_val = (1 << bit_left) - 1;
    else
        new_val = 0;

    put_ui(new_val, bit_left);
}

void vaBitstream::rbspTrailingBits()
{
    put_ui( 1, 1);
    byteAlign(0);
}

void vaBitstream::startCodePrefix()
{
    put_ui( 0x00000001, 32);
}

void vaBitstream::nalHeader(int nal_ref_idc, int nal_unit_type)
{
    put_ui(0, 1);                /* forbidden_zero_bit: 0 */
    put_ui(nal_ref_idc, 2);
    put_ui(nal_unit_type, 5);
}

//---

// EOF


