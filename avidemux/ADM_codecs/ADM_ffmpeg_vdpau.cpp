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

extern "C" {
#include "ADM_lavcodec.h"
}

#include "ADM_default.h"
#ifdef USE_VDPAU
#include "vdpau/vdpau_x11.h"
#include "vdpau/vdpau.h"
#include "ADM_codecs/ADM_codec.h"
#include "ADM_codecs/ADM_ffmp43.h"
#include "DIA_coreToolkit.h"
#include "ADM_dynamicLoading.h"
#include "ADM_userInterfaces/ADM_render/GUI_render.h"


static bool vdpauWorking=false;

static ADM_LibWrapper vdpauDynaLoader;
static VdpDeviceCreateX11 *ADM_createVdpX11;
static VdpDevice             vdpDevice;
static VdpGetProcAddress     *vdpProcAddress;

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


/**
    \fn vdpauProbe
    \brief Try loading vdpau...
*/
bool vdpauProbe(void)
{
    if(false==vdpauDynaLoader.loadLibrary("/usr/lib/libvdpau.so"))
    {
        return false;
    }
    ADM_createVdpX11=(VdpDeviceCreateX11*)vdpauDynaLoader.getSymbol("vdp_device_create_x11");
    if(!ADM_createVdpX11) return false;

    //
    GUI_WindowInfo xinfo;
    void *draw;
    draw=UI_getDrawWidget();
    UI_getWindowInfo(draw,&xinfo );
    
    // try to create....
    if( VDP_STATUS_OK!=ADM_createVdpX11((Display*)xinfo.display,0,&vdpDevice,&vdpProcAddress))
    {
        return false;
    }
    // Now that we have the vdpProcAddress, time to get the functions....
    


    return true;
}
/**

*/
int ADM_getBuffer(AVCodecContext *avctx, AVFrame *pic)
{
    uint32_t w= avctx->width;
    uint32_t h= avctx->height;

    uint32_t rounded_w=(w+63)&~63;
    uint32_t rounded_h=(h+63)&~63;
    uint32_t page=rounded_w*rounded_h;

    pic->data[0]=new uint8_t [page];
    pic->data[1]=new uint8_t [page/4];
    pic->data[2]=new uint8_t [page/4];

    pic->linesize[0]=rounded_w;
    pic->linesize[1]=rounded_w/2;
    pic->linesize[2]=rounded_w/2;
    pic->type= FF_BUFFER_TYPE_USER;
    pic->age=1;
    return 0;
}

 void ADM_releaseBuffer(struct AVCodecContext *avctx, AVFrame *pic)
{
    #define F(x) if(pic->x) delete [] pic->x;pic->x=NULL;
    F(data[0]);
    F(data[1]);
    F(data[2]);
    return;
}
//***************************

decoderFFVDPAU::decoderFFVDPAU(uint32_t w, uint32_t h, uint32_t l, uint8_t * d):decoderFF (w,	   h)
{
        _context->get_buffer      = ADM_getBuffer;
        _context->release_buffer  = ADM_releaseBuffer;
        _context->reget_buffer    = ADM_getBuffer;

        WRAP_OpenByName(h264_vdpau,CODEC_ID_H264);
}


#endif
// EOF