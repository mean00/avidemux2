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
#ifndef ADM_ffmpeg_xvba_internal_H
#define ADM_ffmpeg_xvba_internal_H
#include <vector>


#define NB_SURFACE 25
typedef struct 
{
        //VdpDecoder            vdpDecoder;
        //vdpau_render_state *renders[NB_SURFACE];
        //std::vector <vdpau_render_state *>freeQueue;

}xvbaContext;

#define XVBAX ((xvbaContext *)xvba)


#define WRAP_Open_Template(funcz,argz,display,codecid) \
{\
AVCodec *codec=funcz(argz);\
if(!codec) {GUI_Error_HIG("Codec",QT_TR_NOOP("Internal error finding codec"display));ADM_assert(0);} \
  codecId=codecid; \
  _context->workaround_bugs=1*FF_BUG_AUTODETECT +0*FF_BUG_NO_PADDING; \
  _context->error_concealment=3; \
  if (avcodec_open(_context, codec) < 0)  \
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