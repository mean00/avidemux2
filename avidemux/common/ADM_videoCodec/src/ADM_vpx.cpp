/***************************************************************************
            \file              ADM_ffmpeg_vdpau.cpp  
            \brief Decoder using half ffmpeg/half VDPAU

    The ffmpeg part is to preformat inputs for VDPAU
    VDPAU is loaded dynamically to be able to make a binary
        and have something working even if the target machine
        does not have vdpau
    Some part, especially get/buffer and ip_age borrowed from xbmc
        as the api from ffmpeg is far from clear....


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
#include "config.h"
#ifdef USE_VPX
#include "ADM_vpx.h"

/**
    \fn ctor
*/
decoderVPX::decoderVPX (uint32_t w, uint32_t h,uint32_t fcc, uint32_t extraDataLen, uint8_t *extraData,uint32_t bpp)
        : decoders(  w,   h,  fcc,   extraDataLen,  extraData,  bpp)
{   
    alive=false;
}
/**
    \fn dtor
*/
decoderVPX::~decoderVPX ()
{

}
/**
    \fn uncompress
*/
bool    decoderVPX::uncompress (ADMCompressedImage * in, ADMImage * out)
{
    return false;
}
#endif
// EOF
