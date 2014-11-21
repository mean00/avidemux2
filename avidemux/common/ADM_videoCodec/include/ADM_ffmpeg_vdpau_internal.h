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


#define WRAP_Open_Template(funcz,argz,display,codecid) \
{\
AVCodec *codec=funcz(argz);\
if(!codec) {GUI_Error_HIG("Codec",QT_TR_NOOP("Internal error finding codec"display));ADM_assert(0);} \
  codecId=codecid; \
  _context->workaround_bugs=1*FF_BUG_AUTODETECT +0*FF_BUG_NO_PADDING; \
  _context->error_concealment=3; \
  if (avcodec_open2(_context, codec, NULL) < 0)  \
                      { \
                                        printf("[lavc] Decoder init: "display" video decoder failed!\n"); \
                                        GUI_Error_HIG("Codec","Internal error opening "display); \
                                        ADM_assert(0); \
                                } \
                                else \
                                { \
                                        printf("[lavc] Decoder init: "display" video decoder initialized! (%s)\n",codec->long_name); \
                                } \
}

#define WRAP_Open(x)            {WRAP_Open_Template(avcodec_find_decoder,x,#x,x);}
#define WRAP_OpenByName(x,y)    {WRAP_Open_Template(avcodec_find_decoder_by_name,#x,#x,y);}

#endif