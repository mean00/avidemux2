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
#ifndef ADM_ffmpeg_vdpa_internal_H
#define ADM_ffmpeg_vdpa_internal_H
#include <vector>
extern "C" {
static void draw(struct AVCodecContext *s,    const AVFrame *src, int offset[4],    int y, int type, int height);
static int ADM_VDPAUgetBuffer(AVCodecContext *avctx, AVFrame *pic);
static void ADM_VDPAUreleaseBuffer(struct AVCodecContext *avctx, AVFrame *pic);
}


#define NB_SURFACE 50
typedef struct 
{
        VdpDecoder            vdpDecoder;
        vdpau_render_state *renders[NB_SURFACE];
        std::vector <vdpau_render_state *>freeQueue;

}vdpauContext;

#define VDPAU ((vdpauContext *)vdpau)

#define FF_SHOW		(FF_DEBUG_VIS_MV_P_FOR+	FF_DEBUG_VIS_MV_B_FOR+FF_DEBUG_VIS_MV_B_BACK)

#define WRAP_Open_Template(funcz,argz,display,codecid) \
{\
AVCodec *codec=funcz(argz);\
if(!codec) {GUI_Error_HIG("Codec",QT_TR_NOOP("Internal error finding codec"display));ADM_assert(0);} \
  codecId=codecid; \
  _context = avcodec_alloc_context3 (codec);\
  ADM_assert (_context);\
  _context->max_b_frames = 0;\
  _context->width = _w;\
  _context->height = _h;\
  _context->pix_fmt = PIX_FMT_YUV420P;\
  _context->debug_mv |= FF_SHOW;\
  _context->debug |= FF_DEBUG_VIS_MB_TYPE + FF_DEBUG_VIS_QP;\
  _context->opaque          = this;\
  _context->get_buffer      = ADM_VDPAUgetBuffer;\
  _context->release_buffer  = ADM_VDPAUreleaseBuffer;\
  _context->draw_horiz_band = draw;\
  _context->slice_flags     = SLICE_FLAG_CODED_ORDER|SLICE_FLAG_ALLOW_FIELD;\
  _context->extradata = (uint8_t *) _extraDataCopy;\
  _context->extradata_size  = (int) extraDataLen;\
  _context->get_format      = vdpauGetFormat;\
  _context->workaround_bugs=1*FF_BUG_AUTODETECT +0*FF_BUG_NO_PADDING; \
  _context->error_concealment=3; \
  if (_setBpp) {\
    _context->bits_per_coded_sample = _bpp;\
  }\
  if (_setFcc) {\
    _context->codec_tag=_fcc;\
    _context->stream_codec_tag=_fcc;\
  }\
  if (_extraDataCopy) {\
    _context->extradata = _extraDataCopy;\
    _context->extradata_size = _extraDataLen;\
  }\
  if (_usingMT) {\
    _context->thread_count = _threads;\
  }\
  if (avcodec_open2(_context, codec, NULL) < 0)  \
                      { \
                                        printf("[lavc] Decoder init: "display" video decoder failed!\n"); \
                                        GUI_Error_HIG("Codec","Internal error opening "display); \
                                        ADM_assert(0); \
                                } \
                                else \
                                { \
					_closeCodec = true;\
                                        printf("[lavc] Decoder init: "display" video decoder initialized! (%s)\n",codec->long_name); \
                                } \
}

#define WRAP_Open(x)            {WRAP_Open_Template(avcodec_find_decoder,x,#x,x);}
#define WRAP_OpenByName(x,y)    {WRAP_Open_Template(avcodec_find_decoder_by_name,#x,#x,y);}

#endif
