/***************************************************************************
            \file              ADM_ffmpeg_vdpau.cpp  
            \brief Decoder using half ffmpeg/half VDPAU

    The ffmpeg part is to preformat inputs for VDPAU
    VDPAU is loaded dynamically to be able to make a binary
        and have something working even if the target machine
        does not have vdpau


 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#pragma once
#include <vector>
extern "C" {
static int ADM_VDPAUgetBuffer(AVCodecContext *avctx, AVFrame *pic,int flags);
static void ADM_VDPAUreleaseBuffer(void *opaque, uint8_t *data);
}

/**
 */
typedef struct 
{
        std::vector <vdpau_render_state *>freeQueue;
        std::vector <vdpau_render_state *>fullQueue;


}vdpauContext;

#define VDPAU ((vdpauContext *)vdpau)

#define WRAP_Open_TemplateVdpauByName(argz,codecid) \
    WRAP_Open_Template(avcodec_find_decoder_by_name,argz,,codecid,{\
            _context->opaque          = this;\
            _context->slice_flags     = SLICE_FLAG_CODED_ORDER|SLICE_FLAG_ALLOW_FIELD;\
            _context->get_format      = vdpauGetFormat;})

