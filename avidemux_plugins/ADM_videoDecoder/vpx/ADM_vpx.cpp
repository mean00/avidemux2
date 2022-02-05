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
#include "ADM_vpx.h"
#include "vpx/vpx_decoder.h"
#include "vpx/vp8dx.h"
#define VPX ((vpx_codec_ctx_t *)vpx)
/**
    \fn ctor
*/
decoderVPX::decoderVPX (uint32_t w, uint32_t h,uint32_t fcc, uint32_t extraDataLen, uint8_t *extraData,uint32_t bpp)
        : decoders(  w,   h,  fcc,   extraDataLen,  extraData,  bpp)
{   
    alive=false;
    vpx=NULL;
    vpx_codec_dec_cfg_t cfg;
    vpx_codec_flags_t flags=0; //VPX_CODEC_USE_POSTPROC
    vpx_codec_ctx_t *instance=new vpx_codec_ctx_t;
    const struct vpx_codec_iface *iface;
    if(fcc==MKFCC('V','P','8',' '))
    {
        iface = &vpx_codec_vp8_dx_algo;
    }else if(fcc==MKFCC('V','P','9',' '))
    {
        iface = &vpx_codec_vp9_dx_algo;
    }else
    {
        ADM_warning("Unsupported FCC\n");
        delete instance;
        return;
    }

    memset(instance,0,sizeof(*instance));
    memset(&cfg,0,sizeof(cfg));
    cfg.threads=1;
    cfg.w=w;
    cfg.h=h;
    if(VPX_CODEC_OK!=vpx_codec_dec_init(instance, iface, &cfg, flags))
    {
        delete instance;
        ADM_warning("Vpx init ko\n");
    }else
    {
        alive=true;
        vpx=(void *)instance;
        ADM_info("Vpx init ok\n");

    }
}
/**
    \fn dtor
*/
decoderVPX::~decoderVPX ()
{
    if(vpx)
    {
        vpx_codec_ctx_t *a=VPX;
        delete a;
        vpx=NULL;
    }
    ADM_info("Destroying VPX decoder\n");
}
/**
    \fn uncompress
*/
bool    decoderVPX::uncompress (ADMCompressedImage * in, ADMImage * out)
{
    if (vpx_codec_decode(VPX, in->data, in->dataLength, NULL, 0) != VPX_CODEC_OK) 
    {
        ADM_warning("Error decoding VPX\n");
        return false;
    }
    struct vpx_image *img;
    const void *iter = NULL;
    img = vpx_codec_get_frame(VPX, &iter);
    if(img)
    {
            if (img->fmt != VPX_IMG_FMT_I420) 
            {
                ADM_warning("Wrong Colorspace\n");
                return false;
            }
            ADMImageRef    *r=out->castToRef();
            if(r)
            {
                    r->_planes[0]=img->planes[0];
                    r->_planes[1]=img->planes[2];
                    r->_planes[2]=img->planes[1];
                    r->_planeStride[0]=img->stride[0];
                    r->_planeStride[1]=img->stride[2];
                    r->_planeStride[2]=img->stride[1];
                    r->_pixfrmt=ADM_PIXFRMT_YV12;
                    r->Pts=in->demuxerPts;
                    r->flags=in->flags;
                    r->_width = img->w;
                    r->_height = img->h;
                    return true;
            }
                
            ADM_warning("Only ref for VPX decoder\n");

    }
    return false;
}
// EOF
