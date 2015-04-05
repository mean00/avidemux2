/***************************************************************************
            \file              ADM_ffmpeg_xvba_internal.h
            \brief             ffmpeg<->xvba<->app 
    

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


#define NB_SURFACE 25
#define WRAP_Open_TemplateLibVAByName(codecs) \
 WRAP_Open_Template(avcodec_find_decoder,codecs,#codecs,codecs,{\
            _context->opaque          = this; \
            _context->thread_count    = 1; \
            _context->get_buffer      = ADM_LIBVAgetBuffer; \
            _context->release_buffer  = ADM_LIBVAreleaseBuffer ;    \
            _context->draw_horiz_band = NULL; \
            _context->get_format      = ADM_LIBVA_getFormat; \
            _context->slice_flags     = SLICE_FLAG_CODED_ORDER|SLICE_FLAG_ALLOW_FIELD; \
            _context->pix_fmt         = AV_PIX_FMT_VAAPI_VLD; \
            _context->hwaccel_context = va_context;})


