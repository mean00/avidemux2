/***************************************************************************
            \file  ADM_aomDec.cpp
            \brief AV1 decoder using libaom

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
#include "ADM_aomDec.h"
#include "aom/aom_decoder.h"
#include "aom/aomdx.h"
#define AX ((aom_codec_ctx_t *)cookie)

static ADM_colorPrimaries mapColPriFromAom(aom_color_primaries_t color_primaries);
static ADM_colorTrC mapColTrcFromAom(aom_transfer_characteristics_t color_trc);
static ADM_colorSpace mapColSpcFromAom(aom_matrix_coefficients_t colorspace);

/**
    \fn ctor
*/
decoderAom::decoderAom(uint32_t w, uint32_t h, uint32_t fcc, uint32_t extraDataLen, uint8_t *extraData, uint32_t bpp)
    : decoders(w,h,fcc,extraDataLen,extraData,bpp)
{
    drain=false;
    alive=false;
    cookie=NULL;
    aom_codec_dec_cfg_t cfg;
    aom_codec_flags_t flags=0;
    aom_codec_ctx_t *instance=new aom_codec_ctx_t;
    if(fcc!=MKFCC('a','v','0','1'))
    {
        ADM_warning("Unsupported FCC\n");
        delete instance;
        return;
    }

    memset(instance,0,sizeof(*instance));
    memset(&cfg,0,sizeof(cfg));
    cfg.threads=ADM_cpu_num_processors();
    cfg.w=w;
    cfg.h=h;
    cfg.allow_lowbitdepth=1;
    aom_codec_err_t err=aom_codec_dec_init(instance, &aom_codec_av1_dx_algo, &cfg, flags);
    if(err!=AOM_CODEC_OK)
    {
        delete instance;
        ADM_warning("libaom decoder init failed with error %d: %s.\n",(int)err,aom_codec_err_to_string(err));
    }else
    {
        alive=true;
        cookie=(void *)instance;
        ADM_info("libaom decoder init succeeded, threads: %d\n",cfg.threads);
    }
}
/**
    \fn dtor
*/
decoderAom::~decoderAom()
{
    ADM_info("Destroying libaom decoder.\n");
    if(cookie)
    {
        aom_codec_ctx_t *a=AX;
        aom_codec_destroy(a);
        delete a;
        cookie=NULL;
    }
}
/**
    \fn flush
*/
bool decoderAom::flush(void)
{
    drain=false;
    return true;
}
/**
    \fn uncompress
*/
bool decoderAom::uncompress(ADMCompressedImage *in, ADMImage *out)
{
    aom_codec_err_t err;
    if(drain)
        err=aom_codec_decode(AX, NULL, 0, NULL);
    else
        err=aom_codec_decode(AX, in->data, in->dataLength, NULL);
    if(err!=AOM_CODEC_OK)
    {
        ADM_warning("Error %d (%s) decoding AV1 frame.\n",(int)err,aom_codec_err_to_string(err));
        return false;
    }
    aom_image_t *img;
    const void *iter = NULL;
    img = aom_codec_get_frame(AX, &iter);
    if(drain)
        ADM_info("Draining AOM decoder, %s.\n", img? "delayed picture received" : "no delayed pictures left");
    if(img)
    {
        ADM_pixelFormat pixfrmt = ADM_PIXFRMT_YV12;
        switch(img->fmt)
        {
            case AOM_IMG_FMT_I420:
                if(img->bit_depth == 8) break;
                ADM_warning("Unsupported bit depth %u for AOM_IMG_FMT_I420 image format.\n",img->bit_depth);
                return false;
            case AOM_IMG_FMT_I42016:
                if(img->bit_depth == 10)
                {
                    pixfrmt = ADM_PIXFRMT_YUV420_10BITS;
                    break;
                }
                ADM_warning("Unsupported bit depth %u for AOM_IMG_FMT_I42016 image format.\n",img->bit_depth);
                return false;
            default:
                ADM_warning("Unsupported pixel format 0x%x, bit depth: %u\n",(int)img->fmt,img->bit_depth);
                return false;
        }
        ADMImageRef *r=out->castToRef();
        if(r)
        {
            int u=1,v=2;
            if(pixfrmt != ADM_PIXFRMT_YV12)
            {
                u = 2;
                v = 1;
            }
            r->_planes[0]=img->planes[0];
            r->_planes[v]=img->planes[1];
            r->_planes[u]=img->planes[2];
            r->_planeStride[0]=img->stride[0];
            r->_planeStride[v]=img->stride[1];
            r->_planeStride[u]=img->stride[2];
            r->_pixfrmt=pixfrmt;
            r->_range = (img->range == AOM_CR_FULL_RANGE)? ADM_COL_RANGE_JPEG : ADM_COL_RANGE_MPEG;
            r->_colorPrim = mapColPriFromAom(img->cp);
            r->_colorTrc = mapColTrcFromAom(img->tc);
            r->_colorSpace = mapColSpcFromAom(img->mc);

            r->Pts=in->demuxerPts;
            r->flags=in->flags;
            // make sure the output is not marked as a hw image
            int count = 0;
            while(r->refType != ADM_HW_NONE && count < 32 /* arbitrary limit */)
            {
                r->hwDecRefCount();
                count++;
            }
            return true;
        }
        ADM_warning("AV1 decoder accepts ref image only.\n");
    }
    return false;
}

ADM_colorPrimaries mapColPriFromAom(aom_color_primaries_t color_primaries)
{
    switch(color_primaries)
    {
        case AOM_CICP_CP_BT_709:
            return ADM_COL_PRI_BT709;
        case AOM_CICP_CP_BT_470_M:
            return ADM_COL_PRI_BT470M;
        case AOM_CICP_CP_BT_470_B_G:
            return ADM_COL_PRI_BT470BG;
        case AOM_CICP_CP_BT_601:
            return ADM_COL_PRI_SMPTE170M;
        case AOM_CICP_CP_SMPTE_240:
            return ADM_COL_PRI_SMPTE240M;
        case AOM_CICP_CP_GENERIC_FILM:
            return ADM_COL_PRI_FILM;
        case AOM_CICP_CP_BT_2020:
            return ADM_COL_PRI_BT2020;
        case AOM_CICP_CP_XYZ:
            return ADM_COL_PRI_SMPTE428;
        case AOM_CICP_CP_SMPTE_431:
            return ADM_COL_PRI_SMPTE431;
        case AOM_CICP_CP_SMPTE_432:
            return ADM_COL_PRI_SMPTE432;
        case AOM_CICP_CP_EBU_3213:
            return ADM_COL_PRI_EBU3213;
        default:
            return ADM_COL_PRI_UNSPECIFIED;
    }
}

ADM_colorTrC mapColTrcFromAom(aom_transfer_characteristics_t color_trc)
{
    switch(color_trc)
    {
        case AOM_CICP_TC_BT_709:
            return ADM_COL_TRC_BT709;
        case AOM_CICP_TC_BT_470_M:
            return ADM_COL_TRC_GAMMA22;
        case AOM_CICP_TC_BT_470_B_G:
            return ADM_COL_TRC_GAMMA28;
        case AOM_CICP_TC_BT_601:
            return ADM_COL_TRC_SMPTE170M;
        case AOM_CICP_TC_SMPTE_240:
            return ADM_COL_TRC_SMPTE240M;
        case AOM_CICP_TC_LINEAR:
            return ADM_COL_TRC_LINEAR;
        case AOM_CICP_TC_LOG_100:
            return ADM_COL_TRC_LOG;
        case AOM_CICP_TC_LOG_100_SQRT10:
            return ADM_COL_TRC_LOG_SQRT;
        case AOM_CICP_TC_IEC_61966:
            return ADM_COL_TRC_IEC61966_2_4;
        case AOM_CICP_TC_BT_1361:
            return ADM_COL_TRC_BT1361_ECG;
        case AOM_CICP_TC_SRGB:
            return ADM_COL_TRC_IEC61966_2_1;
        case AOM_CICP_TC_BT_2020_10_BIT:
            return ADM_COL_TRC_BT2020_10;
        case AOM_CICP_TC_BT_2020_12_BIT:
            return ADM_COL_TRC_BT2020_12;
        case AOM_CICP_TC_SMPTE_2084:
            return ADM_COL_TRC_SMPTE2084;
        case AOM_CICP_TC_SMPTE_428:
            return ADM_COL_TRC_SMPTE428;
        case AOM_CICP_TC_HLG:
            return ADM_COL_TRC_ARIB_STD_B67;
        default:
            return ADM_COL_TRC_UNSPECIFIED;
    }
}

ADM_colorSpace mapColSpcFromAom(aom_matrix_coefficients_t colorspace)
{
    switch(colorspace)
    {
        case AOM_CICP_MC_IDENTITY:
            return ADM_COL_SPC_sRGB;
        case AOM_CICP_MC_BT_709:
            return ADM_COL_SPC_BT709;
        case AOM_CICP_MC_FCC:
            return ADM_COL_SPC_FCC;
        case AOM_CICP_MC_BT_470_B_G:
            return ADM_COL_SPC_BT470BG;
        case AOM_CICP_MC_BT_601:
            return ADM_COL_SPC_SMPTE170M;
        case AOM_CICP_MC_SMPTE_240:
            return ADM_COL_SPC_SMPTE240M;
        case AOM_CICP_MC_SMPTE_YCGCO:
            return ADM_COL_SPC_YCGCO;
        case AOM_CICP_MC_BT_2020_NCL:
            return ADM_COL_SPC_BT2020_NCL;
        case AOM_CICP_MC_BT_2020_CL:
            return ADM_COL_SPC_BT2020_CL;
        case AOM_CICP_MC_SMPTE_2085:
            return ADM_COL_SPC_SMPTE2085;
        case AOM_CICP_MC_CHROMAT_NCL:
            return ADM_COL_SPC_CHROMA_DERIVED_NCL;
        case AOM_CICP_MC_CHROMAT_CL:
            return ADM_COL_SPC_CHROMA_DERIVED_CL;
        case AOM_CICP_MC_ICTCP:
            return ADM_COL_SPC_ICTCP;
        default:
            return ADM_COL_SPC_UNSPECIFIED;
    }
}

// EOF
