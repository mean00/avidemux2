/***************************************************************************
                          ADM_rgb16.cpp  -  description
                             -------------------
    begin                : Mon May 27 2002
    copyright            : (C) 2002 by mean
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

#include "ADM_default.h"
#include "ADM_codec.h"
#include "ADM_rgb16.h"
/**
    \fn ctor
*/

decoderRGB16::decoderRGB16(uint32_t w, uint32_t h,uint32_t fcc, uint32_t extraDataLen, uint8_t *extraData,uint32_t bpp)
    : decoders (  w,   h,  fcc,   extraDataLen,   extraData,  bpp)
{
    _bpp = bpp;
    bytePerPixel=_bpp>>3;
    decoded = new uint8_t[2*bytePerPixel * w * h];
}
/**
    \fn dtor
*/
decoderRGB16::~decoderRGB16()
{
    delete[] decoded;
    decoded=NULL;
}
/**
    \fn uncompress
*/
bool decoderRGB16::uncompress(ADMCompressedImage * in, ADMImage * out)
{
        int lineSize = (_w *bytePerPixel + 3) & ~3; // 4 bytes aligned ?
        ADM_colorspace colorspace;
        int i, j;
        uint8_t *src = in->data;
        uint8_t *dst = decoded;
        int      outBytePerPixel=bytePerPixel;
        switch (_bpp)
        {
                case 16:
                        // FIXME - 16-bit could use a BGR555 or BGR565 colour mask
                        colorspace = ADM_COLOR_BGR555;
                        break;
                case 24:
                case 32:
                        colorspace = ADM_COLOR_RGB24;
                        break;
                default:
                        printf("bpp %d not supported\n", _bpp);
                        return false;
        }
    // Pack...
        // Invert scanline
        src = in->data+lineSize*(_h-1);
        if (_bpp == 32) // 32 -> 24
        {
            outBytePerPixel=3;           
            for(i = 0; i < _h; i++)
            {
                    uint8_t *buf = src;
                    uint8_t *ptr = dst;

                    for(j = 0; j < _w; j++)
                    {
                            ptr[0] = buf[0]; // remove alpha channel + reorder. IT would be more efficient to do it in colorspace...
                            ptr[1] = buf[1];
                            ptr[2] = buf[2];
                            ptr += 3;
                            buf += 4;
                    }
                    src -= lineSize;
                    dst += _w * 3;                    
            }
        }
        else // 24/16/8 bpp
        {           
            for(int i=0;i<_h;i++)
            {                
                memcpy(dst, src, _w * bytePerPixel);
                src -= lineSize;
                dst += _w * bytePerPixel;
            }
        }

        ADM_assert(out->isRef());
        ADMImageRef *ref=out->castToRef();
        out->flags = AVI_KEY_FRAME;
        out->_colorspace = colorspace;

        ref->_planes[0] = decoded;
        ref->_planes[1] = NULL;
        ref->_planes[2] = NULL;

        ref->_planeStride[0] = outBytePerPixel * _w;
        ref->_planeStride[1] = 0;
        ref->_planeStride[2] = 0;
        out->Pts=in->demuxerPts;
        return true;
}
//EOF
