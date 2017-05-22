/***************************************************************************
    \file  ADM_ffVp9
    \brief Decoders using lavcodec
    \author mean & all (c) 2017
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <stddef.h>

#include "ADM_default.h"
#include "ADM_ffmp43.h"
#include "DIA_coreToolkit.h"
#include "ADM_hwAccel.h"
#include "ADM_codecFFVP9.h"
/**
 * \fn ctor
 */
decoderFFVP9::decoderFFVP9 (uint32_t w, uint32_t h,uint32_t fcc, uint32_t extraDataLen, uint8_t *extraData,uint32_t bpp)
    : decoderFFSimple(w,h,fcc,extraDataLen,extraData,bpp)
{
    
}
/**
 * \fn dtor
 */
decoderFFVP9::~decoderFFVP9 ()
{
    
}
/**
 * \fn uncompress
 */
bool    decoderFFVP9::uncompress (ADMCompressedImage * in, ADMImage * out)
{
    return decoderFFSimple::uncompress(in,out);
}
// eof
