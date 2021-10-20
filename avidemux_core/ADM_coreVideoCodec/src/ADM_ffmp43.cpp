/***************************************************************************
    \file ADM_ffmp43
    \brief Decoders using lavcodec
    \author mean & all (c) 2002-2010
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
#include "prefs.h"

#ifdef ADM_DEBUG
    #define LAV_VERBOSITY_LEVEL AV_LOG_DEBUG
#else
    #define LAV_VERBOSITY_LEVEL AV_LOG_INFO
#endif

#define aprintf(...) {}


//****************************
extern uint8_t DIA_lavDecoder (bool *swapUv);
#if 0
extern "C"
{
  int av_is_voppacked (AVCodecContext * avctx, int *vop_packed, int *gmc,
		       int *qpel);
};
#endif
/**
    \fn clonePic
    \brief Convert AvFrame to ADMImage
*/
uint8_t decoderFF::clonePic (AVFrame * src, ADMImage * out, bool swap)
{
  uint32_t    u,v;
  ADM_assert(out->isRef());
  ADMImageRef *ref=out->castToRef();
  ref->_planes[0] = (uint8_t *) src->data[0];
  ref->_planeStride[0] = src->linesize[0];
  swap = swap != decoderFF_params.swapUv;
  if (swap)
    {
      u = 1;
      v = 2;
    }
  else
    {
      u = 2;
      v = 1;
    }
  ref->_planes[1] = (uint8_t *) src->data[v];
  ref->_planeStride[1] = src->linesize[v];

  ref->_planes[2] = (uint8_t *) src->data[u];
  ref->_planeStride[2] = src->linesize[u];

  // out->_Qp = (src->quality * 32) / FF_LAMBDA_MAX;
  out->flags = frameType ();

#if 0 /* deprecated and dead, removed upstream */
  // Quant ?
  if (src->qstride && src->qscale_table && codecId != AV_CODEC_ID_H264)
    {
      out->quant = (uint8_t *) src->qscale_table;
      out->_qStride = src->qstride;
      out->_qSize = (_w + 15) >> 4;
      out->_qSize *= (_h + 15) >> 4;	// FixME?
    }
  else
#endif
    {
      out->_qSize = out->_qStride = 0;
      out->quant = NULL;
    }
    uint64_t pts_opaque=(uint64_t)(src->reordered_opaque);
    //printf("[LAVC] Old pts :%"PRId64" new pts :%"PRId64"\n",out->Pts, pts_opaque);
    //printf("[LAVC] pts: %"PRIu64"\n",src->pts);
    out->Pts= (uint64_t)(pts_opaque);
    out->_range=(src->color_range==AVCOL_RANGE_JPEG)? ADM_COL_RANGE_JPEG : ADM_COL_RANGE_MPEG;
    {
    ADM_colorPrimaries cp = admColPriFromLav(src->color_primaries);
    if (cp != ADM_COL_PRI_UNSPECIFIED)
        out->_colorPrim = cp;
    ADM_colorTrC tc = admColTrcFromLav(src->color_trc);
    if (tc != ADM_COL_TRC_UNSPECIFIED)
        out->_colorTrc = tc;
    ADM_colorSpace cs = admColSpcFromLav(src->colorspace);
    if (cs != ADM_COL_SPC_UNSPECIFIED)
        out->_colorSpace = cs;
    }
#ifdef ADM_DEBUG
    if (out->_colorPrim == ADM_COL_PRI_UNSPECIFIED)
        ADM_warning("Unspecified color primaries\n");
    if (out->_colorTrc == ADM_COL_TRC_UNSPECIFIED)
        ADM_warning("Unspecified color transfer characteristic\n");
    if (out->_colorSpace == ADM_COL_SPC_UNSPECIFIED)
        ADM_warning("Unspecified colorspace\n");
#endif

    // process side data
    for (int i = 0; i < src->nb_side_data; i++)
    {
        AVFrameSideData *sd = src->side_data[i];
        switch (sd->type)
        {
            case AV_FRAME_DATA_MASTERING_DISPLAY_METADATA:
                {
                    const AVMasteringDisplayMetadata *mastering_display;
                    if (sd->size < sizeof(AVMasteringDisplayMetadata))
                        break;
                    mastering_display = (const AVMasteringDisplayMetadata *)sd->data;
                    if (mastering_display->has_primaries)
                    {
                        for (int rgb=0; rgb<3; rgb++)
                        {
                            out->_hdrInfo.primaries[rgb][0] = av_q2d(mastering_display->display_primaries[rgb][0]);
                            out->_hdrInfo.primaries[rgb][1] = av_q2d(mastering_display->display_primaries[rgb][1]);
                        }
                        out->_hdrInfo.whitePoint[0] = av_q2d(mastering_display->white_point[0]);
                        out->_hdrInfo.whitePoint[1] = av_q2d(mastering_display->white_point[1]);
#ifdef ADM_DEBUG
                        ADM_info("HDR primaries: [%f,%f], [%f,%f], [%f,%f]\n",out->_hdrInfo.primaries[0][0],out->_hdrInfo.primaries[0][1], out->_hdrInfo.primaries[1][0],out->_hdrInfo.primaries[1][1], out->_hdrInfo.primaries[2][0],out->_hdrInfo.primaries[2][1]);
                        ADM_info("HDR white point: [%f,%f]\n",out->_hdrInfo.whitePoint[0],out->_hdrInfo.whitePoint[1]);
#endif
                    }
                    if (mastering_display->has_luminance)
                    {
                        out->_hdrInfo.maxLuminance = av_q2d(mastering_display->max_luminance);	// nits
                        out->_hdrInfo.minLuminance = av_q2d(mastering_display->min_luminance);
#ifdef ADM_DEBUG
                        ADM_info("HDR max luminance: %f\n",out->_hdrInfo.maxLuminance);
                        ADM_info("HDR min luminance: %f\n",out->_hdrInfo.minLuminance);
#endif
                    }
                }
                break;
            case AV_FRAME_DATA_CONTENT_LIGHT_LEVEL:
                {
                    const AVContentLightMetadata *content_light;
                    if (sd->size < sizeof(AVContentLightMetadata))
                        break;
                    content_light = (const AVContentLightMetadata *)sd->data;
                    out->_hdrInfo.maxCLL = content_light->MaxCLL;	// integer, nits
                    out->_hdrInfo.maxFALL = content_light->MaxFALL;
#ifdef ADM_DEBUG
                    ADM_info("HDR maxCLL: %f\n",out->_hdrInfo.maxCLL);
                    ADM_info("HDR maxFALL: %f\n",out->_hdrInfo.maxFALL);
#endif
                }
                break;
            case AV_FRAME_DATA_DYNAMIC_HDR_PLUS:
                {
                    const AVDynamicHDRPlus *hdr_plus;
                    if (sd->size < sizeof(AVDynamicHDRPlus))
                        break;
                    hdr_plus = (const AVDynamicHDRPlus *)sd->data;
                    if (hdr_plus->num_windows > 0)
                    {
                        // CHEATING: assume num_windows==1; ATSC S34-301r2 A/341 Amendment – 2094-40 says: "The value of num_windows shall be 1."
                        //           ignore bullshit parameters, like "elliptical pixel selector"; hope it's ok :)
                        for (int rgb=0; rgb<3; rgb++)
                        {
                            out->_hdrInfo.maxSCL[rgb] = av_q2d(hdr_plus->params[0].maxscl[rgb]);
                        }
                        out->_hdrInfo.avgMaxRGB = av_q2d(hdr_plus->params[0].average_maxrgb);
                        if (hdr_plus->params[0].tone_mapping_flag)
                        {
                            out->_hdrInfo.kneePoint[0] = av_q2d(hdr_plus->params[0].knee_point_x);
                            out->_hdrInfo.kneePoint[1] = av_q2d(hdr_plus->params[0].knee_point_y);
                        }
                        if (hdr_plus->params[0].num_bezier_curve_anchors > 0)
                        {
                            for (int b=0; b<hdr_plus->params[0].num_bezier_curve_anchors; b++)
                            {
                                if (b >= 15)
                                    break;
                                out->_hdrInfo.bezierCurveAnchors[b] = av_q2d(hdr_plus->params[0].bezier_curve_anchors[b]);
                            }
                        }
                        if (hdr_plus->params[0].color_saturation_mapping_flag)
                        {
                            out->_hdrInfo.colorSaturationWeight = av_q2d(hdr_plus->params[0].color_saturation_weight);
                        }
                    }
                    out->_hdrInfo.targetMaxLuminance = av_q2d(hdr_plus->targeted_system_display_maximum_luminance);
#ifdef ADM_DEBUG
                    ADM_info("HDR maxSCL: %f, %f, %f\n",out->_hdrInfo.maxSCL[0],out->_hdrInfo.maxSCL[1],out->_hdrInfo.maxSCL[2]);
                    ADM_info("HDR avgMaxRGB: %f\n",out->_hdrInfo.avgMaxRGB);
                    ADM_info("HDR kneePoint: %f, %f\n",out->_hdrInfo.kneePoint[0],out->_hdrInfo.kneePoint[1]);
                    ADM_info("HDR colorSaturationWeight: %f\n",out->_hdrInfo.colorSaturationWeight);
                    ADM_info("HDR targetMaxLuminance: %f\n",out->_hdrInfo.targetMaxLuminance);
#endif

                    //other fields are not implemented as of ATSC S34-301r2 A/341 Amendment – 2094-40 
                }
                break;
        }
    }

    return 1;
}
/**
        \fn decoderMultiThread
        \brief Enabled multitheaded decoder if possible
*/
void decoderFF::decoderMultiThread (void)
{
    static uint32_t sessionThreads = 0;
    uint32_t threads = 1;

    if(false == prefs->get(FEATURES_THREADING_LAVC, &threads))
        threads = 1;
    if(!threads)
        threads = ADM_cpu_num_processors();
    if(threads > LAVC_MAX_SAFE_THREAD_COUNT)
        threads = LAVC_MAX_SAFE_THREAD_COUNT;
    if(!sessionThreads)
    {
        sessionThreads = threads;
    }else
    {
        if((threads > 1) != (sessionThreads > 1))
            ADM_warning("Restart application to %s multithreaded decoding.\n",(threads>1)? "enable" : "disable");
        else
            sessionThreads = threads;
    }
    if(sessionThreads > 1)
    {
        printf ("[lavc] Enabling MT decoder with %u threads\n", sessionThreads);
        _threads = sessionThreads;
        _usingMT = 1;
    }
}
uint8_t decoderFF::getPARWidth (void)
{
  if(!_context->sample_aspect_ratio.num) return 1;
  return _context->sample_aspect_ratio.num;
}
uint8_t decoderFF::getPARHeight (void)
{
  if(!_context->sample_aspect_ratio.den) return 1;
  return _context->sample_aspect_ratio.den;

}

//________________________________________________
bool  decoderFF::setParam(void)
{
    DIA_lavDecoder(&decoderFF_params.swapUv);
    return true;
}

const decoderFF::decoderFF_param_t decoderFF::defaultConfig = {false};

const ADM_paramList decoderFF::decoderFF_param_template[] =
{
	{"swapUv", offsetof(decoderFF_param_t, swapUv), "bool", ADM_param_bool},
	{NULL, 0, NULL}
};

bool decoderFF::getConfiguration(CONFcouple **conf)
{
	return ADM_paramSave(conf, decoderFF_param_template, &decoderFF_params);
}

bool decoderFF::resetConfiguration()
{
	memcpy(&decoderFF_params, &defaultConfig, sizeof(decoderFF_param_t));

	return true;
}

bool decoderFF::setConfiguration(CONFcouple * conf)
{
	return ADM_paramLoad(conf, decoderFF_param_template, &decoderFF_params);
}

//-------------------------------
decoderFF::decoderFF (uint32_t w, uint32_t h,uint32_t fcc, uint32_t extraDataLen, uint8_t *extraData,uint32_t bpp)
            :decoders (w, h,fcc,extraDataLen,extraData,bpp)
{
	resetConfiguration();
  hurryUp=false;
  _initCompleted=false;
  _drain=false;
  _done=false;
  _keepFeeding=false;
  _endOfStream=false;
  _setBpp=false;
  _setFcc=false;
  codecId = 0;
//                              memset(&_context,0,sizeof(_context));
  _allowNull = 0;
  _gmc = 0;
  _context = NULL;
  _frame = NULL;
  _refCopy = 0;
  _usingMT = 0;
  _bpp = bpp;
  _fcc = fcc;

  _frame=av_frame_alloc();
  if(!_frame)
      return;

  _packet = av_packet_alloc();
  if(!_packet)
      return;

  printf ("[lavc] Build: %d\n", LIBAVCODEC_BUILD);
  _extraDataCopy=NULL;
  
  if(extraDataLen)
    {
            _extraDataLen=(int)extraDataLen;
            _extraDataCopy=new uint8_t[extraDataLen+AV_INPUT_BUFFER_PADDING_SIZE];
            memset(_extraDataCopy,0,extraDataLen+AV_INPUT_BUFFER_PADDING_SIZE);
            memcpy(_extraDataCopy,extraData,extraDataLen);
    }
   hwDecoder=NULL;

}

//_____________________________________________________

decoderFF::~decoderFF ()
{
  if (_usingMT)
    {
      printf ("[lavc] Killing decoding threads\n");
      _usingMT = 0;
    }
  if(_context)
  {
        avcodec_close (_context);
        av_free(_context);
        _context=NULL;
        printf ("[lavc] Destroyed\n");
    }
  // NULL-safe and will set the pointer to NULL
  av_frame_free(&_frame);
  av_packet_free(&_packet);

  if(_extraDataCopy)
  {
      delete [] _extraDataCopy;
      _extraDataCopy=NULL;
  }
  delete hwDecoder;
  hwDecoder=NULL;
}
/**
 * \fn initialized
 */
bool decoderFF::initialized(void)
{
    return _initCompleted;
}


ADM_pixelFormat decoderFF::admPixFrmtFromLav(AVPixelFormat pix_fmt, bool * swap)
{
  switch (pix_fmt)
    {
    case AV_PIX_FMT_YUV411P:
      aprintf("pixel format is AV_PIX_FMT_YUV411P --> ADM_PIXFRMT_YUV411\n");
      return ADM_PIXFRMT_YUV411;
    case AV_PIX_FMT_YUYV422:
      aprintf("pixel format is AV_PIX_FMT_YUYV422 --> ADM_PIXFRMT_YUV422\n");
      return ADM_PIXFRMT_YUV422;
    case AV_PIX_FMT_YUV422P:
    case AV_PIX_FMT_YUVJ422P:
      aprintf("pixel format is AV_PIX_FMT_YUV422P or AV_PIX_FMT_YUVJ422P --> ADM_PIXFRMT_YUV422P\n");
      return ADM_PIXFRMT_YUV422P;
    case AV_PIX_FMT_GRAY8:
      aprintf("pixel format is AV_PIX_FMT_GRAY8 --> ADM_PIXFRMT_Y8\n");
      return ADM_PIXFRMT_Y8;
    case AV_PIX_FMT_YUV444P:
    case AV_PIX_FMT_YUVJ444P:
      aprintf("pixel format is AV_PIX_FMT_YUV444P or AV_PIX_FMT_YUVJ444P --> ADM_PIXFRMT_YUV444\n");
      return ADM_PIXFRMT_YUV444;
    case AV_PIX_FMT_YUV420P:
    case AV_PIX_FMT_YUVJ420P:
    case AV_PIX_FMT_YUVA420P:
      // Default is YV12 or I420
      // In that case depending on swap u/v
      // we do it or not
      aprintf("pixel format is AV_PIX_FMT_YUV420P, AV_PIX_FMT_YUVJ420P or AV_PIX_FMT_YUVA420P --> ADM_PIXFRMT_YV12\n");
      if (swap)
          *swap = true;
      return ADM_PIXFRMT_YV12;
    case AV_PIX_FMT_BGR24:
      aprintf("pixel format is AV_PIX_FMT_BGR24 --> ADM_PIXFRMT_BGR24\n");
      return ADM_PIXFRMT_BGR24;
    case AV_PIX_FMT_RGB24:
      aprintf("pixel format is AV_PIX_FMT_RGB24 --> ADM_PIXFRMT_RGB24\n");
      return ADM_PIXFRMT_RGB24;
    case AV_PIX_FMT_GBRP:
      aprintf("pixel format is AV_PIX_FMT_GBRP --> ADM_PIXFRMT_GBR24P\n");
      return ADM_PIXFRMT_GBR24P;
    case AV_PIX_FMT_BGR0:
    case AV_PIX_FMT_BGRA:
      aprintf("pixel format is AV_PIX_FMT_BGR0 or AV_PIX_FMT_BGRA --> ADM_PIXFRMT_BGR32A\n");
      return ADM_PIXFRMT_BGR32A;
    case AV_PIX_FMT_RGBA: // ???PIX_FMT_RGBA32:
      aprintf("pixel format is AV_PIX_FMT_RGBA --> ADM_PIXFRMT_RGB32A\n");
      return ADM_PIXFRMT_RGB32A;
    case AV_PIX_FMT_RGB555:
      aprintf("pixel format is AV_PIX_FMT_RGB555 --> ADM_PIXFRMT_RGB555\n");
      return ADM_PIXFRMT_RGB555;
#if 0
    case AV_PIX_FMT_VDPAU_MPEG1:
    case AV_PIX_FMT_VDPAU_MPEG2:
    case AV_PIX_FMT_VDPAU_WMV3:
    case AV_PIX_FMT_VDPAU_VC1:
    case AV_PIX_FMT_VDPAU_H264:
    case AV_PIX_FMT_VDPAU:
        return ADM_PIXFRMT_VDPAU;
    case AV_PIX_FMT_VAAPI:
        return ADM_PIXFRMT_LIBVA;
        
#ifdef USE_XVBA        
    case AV_PIX_FMT_XVBA_VLD:
        return ADM_PIXFRMT_XVBA;
#endif        
#endif
  case AV_PIX_FMT_YUV444P10LE:
        return ADM_PIXFRMT_YUV444_10BITS;
  case AV_PIX_FMT_YUV422P10LE:
        return ADM_PIXFRMT_YUV422_10BITS;
  case   AV_PIX_FMT_P010LE:      
        return ADM_PIXFRMT_NV12_10BITS;
  case  AV_PIX_FMT_YUV420P10LE:
        return ADM_PIXFRMT_YUV420_10BITS;
  case  AV_PIX_FMT_YUV420P12LE:
        return ADM_PIXFRMT_YUV420_12BITS;
  case  AV_PIX_FMT_YUV444P12LE:
        return ADM_PIXFRMT_YUV444_12BITS;
    default:
        return ADM_PIXFRMT_INVALID;
    }
}

ADM_colorPrimaries decoderFF::admColPriFromLav(AVColorPrimaries color_primaries)
{
    switch(color_primaries)
    {
        case AVCOL_PRI_BT709:
            return ADM_COL_PRI_BT709;
        case AVCOL_PRI_BT470M:
            return ADM_COL_PRI_BT470M;
        case AVCOL_PRI_BT470BG:
            return ADM_COL_PRI_BT470BG;
        case AVCOL_PRI_SMPTE170M:
            return ADM_COL_PRI_SMPTE170M;
        case AVCOL_PRI_SMPTE240M:
            return ADM_COL_PRI_SMPTE240M;
        case AVCOL_PRI_FILM:
            return ADM_COL_PRI_FILM;
        case AVCOL_PRI_BT2020:
            return ADM_COL_PRI_BT2020;
        case AVCOL_PRI_SMPTE428:
        //case AVCOL_PRI_SMPTEST428_1:
            return ADM_COL_PRI_SMPTE428;
        case AVCOL_PRI_SMPTE431:
            return ADM_COL_PRI_SMPTE431;
        case AVCOL_PRI_SMPTE432:
            return ADM_COL_PRI_SMPTE432;
        case AVCOL_PRI_EBU3213:
        //case AVCOL_PRI_JEDEC_P22:
            return ADM_COL_PRI_EBU3213;
        default:
            return ADM_COL_PRI_UNSPECIFIED;
    }
}

ADM_colorTrC decoderFF::admColTrcFromLav(AVColorTransferCharacteristic color_trc)
{
    switch(color_trc)
    {
        case AVCOL_TRC_BT709:
            return ADM_COL_TRC_BT709;
        case AVCOL_TRC_GAMMA22:
            return ADM_COL_TRC_GAMMA22;
        case AVCOL_TRC_GAMMA28:
            return ADM_COL_TRC_GAMMA28;
        case AVCOL_TRC_SMPTE170M:
            return ADM_COL_TRC_SMPTE170M;
        case AVCOL_TRC_SMPTE240M:
            return ADM_COL_TRC_SMPTE240M;
        case AVCOL_TRC_LINEAR:
            return ADM_COL_TRC_LINEAR;
        case AVCOL_TRC_LOG:
            return ADM_COL_TRC_LOG;
        case AVCOL_TRC_LOG_SQRT:
            return ADM_COL_TRC_LOG_SQRT;
        case AVCOL_TRC_IEC61966_2_4:
            return ADM_COL_TRC_IEC61966_2_4;
        case AVCOL_TRC_BT1361_ECG:
            return ADM_COL_TRC_BT1361_ECG;
        case AVCOL_TRC_IEC61966_2_1:
            return ADM_COL_TRC_IEC61966_2_1;
        case AVCOL_TRC_BT2020_10:
            return ADM_COL_TRC_BT2020_10;
        case AVCOL_TRC_BT2020_12:
            return ADM_COL_TRC_BT2020_12;
        case AVCOL_TRC_SMPTE2084:
        //case AVCOL_TRC_SMPTEST2084:
            return ADM_COL_TRC_SMPTE2084;
        case AVCOL_TRC_SMPTE428:
        //case AVCOL_TRC_SMPTEST428_1:
            return ADM_COL_TRC_SMPTE428;
        case AVCOL_TRC_ARIB_STD_B67:
            return ADM_COL_TRC_ARIB_STD_B67;
        default:
            return ADM_COL_TRC_UNSPECIFIED;
    }
}

ADM_colorSpace decoderFF::admColSpcFromLav(AVColorSpace colorspace)
{
    switch(colorspace)
    {
        case AVCOL_SPC_RGB:
            return ADM_COL_SPC_sRGB;
        case AVCOL_SPC_BT709:
            return ADM_COL_SPC_BT709;
        case AVCOL_SPC_FCC:
            return ADM_COL_SPC_FCC;
        case AVCOL_SPC_BT470BG:
            return ADM_COL_SPC_BT470BG;
        case AVCOL_SPC_SMPTE170M:
            return ADM_COL_SPC_SMPTE170M;
        case AVCOL_SPC_SMPTE240M:
            return ADM_COL_SPC_SMPTE240M;
        case AVCOL_SPC_YCGCO:
        //case AVCOL_SPC_YCOCG:
            return ADM_COL_SPC_YCGCO;
        case AVCOL_SPC_BT2020_NCL:
            return ADM_COL_SPC_BT2020_NCL;
        case AVCOL_SPC_BT2020_CL:
            return ADM_COL_SPC_BT2020_CL;
        case AVCOL_SPC_SMPTE2085:
            return ADM_COL_SPC_SMPTE2085;
        case AVCOL_SPC_CHROMA_DERIVED_NCL:
            return ADM_COL_SPC_CHROMA_DERIVED_NCL;
        case AVCOL_SPC_CHROMA_DERIVED_CL:
            return ADM_COL_SPC_CHROMA_DERIVED_CL;
        case AVCOL_SPC_ICTCP:
            return ADM_COL_SPC_ICTCP;
        default:
            return ADM_COL_SPC_UNSPECIFIED;
    }
}


/**
 * 
 * @param pic
 * @return 
 */
uint32_t decoderFF::admFrameTypeFromLav (AVFrame *pic)
{
    uint32_t outFlags=0;
#define SET(x)      {outFlags=x;}
#define SET_ADD(x)  {outFlags|=x;}
    
  switch (pic->pict_type)
    {
        case AV_PICTURE_TYPE_B:
                SET (AVI_B_FRAME);
                break;
        case AV_PICTURE_TYPE_I:
                SET (AVI_KEY_FRAME);
                if (!pic->key_frame)
                  {
                    if (codecId == AV_CODEC_ID_H264 || codecId == AV_CODEC_ID_FFV1)
                        SET (AVI_P_FRAME)
                    else
                        ADM_info("Picture type is I, but keyframe is not set\n");
                  }
                break;
        case AV_PICTURE_TYPE_S:
                _gmc = 1;			// No break, just inform that gmc is there
        case AV_PICTURE_TYPE_P:
                SET (AVI_P_FRAME);
                if (pic->key_frame)
                        aprintf ("\n But keyframe is set\n");
                break;
        case AV_PICTURE_TYPE_NONE:
                if(codecId == AV_CODEC_ID_HUFFYUV || codecId == AV_CODEC_ID_FFVHUFF)
                    SET(AVI_KEY_FRAME)
        default:
                break;
    }
    outFlags&=~AVI_STRUCTURE_TYPE_MASK;
    if(pic->interlaced_frame)
    {
        SET_ADD(AVI_FIELD_STRUCTURE)
        if(pic->top_field_first)
            SET_ADD(AVI_TOP_FIELD)
        else
            SET_ADD(AVI_BOTTOM_FIELD)
    }
  return outFlags;
}
/**
    \fn frameType
    \return frametype of the last decoded frame
*/
uint32_t decoderFF::frameType (void)
{
    return admFrameTypeFromLav(_frame);

}
bool decoderFF::decodeHeaderOnly (void)
{
  hurryUp=true;
  _context->skip_frame=AVDISCARD_ALL;
  _context->skip_idct=AVDISCARD_ALL;
  printf ("\n[lavc] Hurry up\n");
  return 1;
}
bool decoderFF::decodeFull (void)
{
  _context->skip_frame=AVDISCARD_DEFAULT;
  _context->skip_idct=AVDISCARD_DEFAULT;
  hurryUp=false;
  printf ("\n[lavc] full decoding\n");
  return 1;
}

/**
    \fn flush
    \brief empty internal buffer
*/
bool    decoderFF::flush(void)
{
    if(_context)
        avcodec_flush_buffers(_context);
    _drain=false;
    _done=false;
    return true;
}

/**
    \fn decodeErrorHandler
    \brief Evaluate return value of avcodec_receive_frame
*/
bool decoderFF::decodeErrorHandler(int code)
{
    if(code<0)
    {
        switch(code)
        {
            case AVERROR_EOF:
                ADM_warning("[lavc] End of video stream reached\n");
                _keepFeeding=false;
                _endOfStream=true;
                flush();
                return false;
            case AVERROR(EAGAIN):
#ifdef ADM_DEBUG
                ADM_info("[lavc] The decoder expects more input before output can be produced\n");
#endif
                _keepFeeding=true;
                return false;
            case AVERROR(EINVAL):
                ADM_error("[lavc] Codec not opened\n");
                return false;
            default:
            {
                char er[AV_ERROR_MAX_STRING_SIZE]={0};
                av_make_error_string(er, AV_ERROR_MAX_STRING_SIZE, code);
                ADM_warning("Error %d in lavcodec (%s)\n",code,er);
                return false;
            }
        }
    }
    _keepFeeding=false;
    _endOfStream=false;
    return true;
}

/**
    \fn uncompress
    \brief Actually decode an image
*/
bool   decoderFF::uncompress (ADMCompressedImage * in, ADMImage * out)
{
  int ret = 0;
  out->_noPicture = 0;
  out->_Qp = ADM_IMAGE_UNKNOWN_QP;
  if(hwDecoder && !_usingMT)
        return hwDecoder->uncompress(in,out);
 
  //printf("Frame size : %d\n",in->dataLength);

    if (!_drain && in->dataLength == 0 && !_allowNull) // Null frame, silently skipped
    {
        printf ("[Codec] null frame\n");
        out->_noPicture = 1;
        out->Pts=ADM_COMPRESSED_NO_PTS;
        printf("[Codec] No Picture\n");
        return 1;
    }
    // Put a safe value....
    out->Pts=in->demuxerPts;
    _context->reordered_opaque=in->demuxerPts;
  //_frame.opaque=(void *)out->Pts;
  //printf("Incoming Pts :%"PRId64"\n",out->Pts);
    if(_drain)
    {
        if(!_done)
        {
            avcodec_send_packet(_context, NULL);
            _done=true;
        }
    }else
    {
        _packet->data = in->data;
        _packet->size = in->dataLength;

        if(in->flags&AVI_KEY_FRAME)
            _packet->flags = AV_PKT_FLAG_KEY;
        else
            _packet->flags = 0;

        ret = avcodec_send_packet(_context, _packet);

        if(ret)
        {
            char er[AV_ERROR_MAX_STRING_SIZE]={0};
            av_make_error_string(er, AV_ERROR_MAX_STRING_SIZE, ret);
            ADM_warning("Ignoring error %d submitting packet to decoder (\"%s\")\n",ret,er);
        }

        av_packet_unref(_packet);

        // HW accel may be setup by now if this has been the very first packet. In this case
        // ask the hw decoder to collect the decoded picture.
        if(hwDecoder)
        {
            hwDecoder->skipSendFrame();
            return hwDecoder->uncompress(in,out);
        }
    }

    ret = avcodec_receive_frame(_context, _frame);

    out->_qStride = 0; // Default = no quant

    if (!_drain && ret && !hurryUp)
    {
        // Some encoder code a vop header with the
        // vop flag set to 0
        // it is meant to mean frame skipped but very dubious
#define MPEG4_EMPTY_FRAME_THRESHOLD 19 // MAX_NVOP_SIZE
#define FRAPS_EMPTY_FRAME_THRESHOLD  8 // assumption
        if ((codecId == AV_CODEC_ID_MPEG4 && in->dataLength <= MPEG4_EMPTY_FRAME_THRESHOLD) ||
            (codecId == AV_CODEC_ID_FRAPS && in->dataLength <= FRAPS_EMPTY_FRAME_THRESHOLD))
        {
            printf ("[lavc] Probably placeholder frame (data length: %u)\n",in->dataLength);
            out->Pts=ADM_NO_PTS; // not sure
            out->_noPicture = 1;
            return true;
        }
        // allow null means we allow null frame in and so potentially
        // have no frame out for a time
        // in that case silently fill with black and returns it as KF
        if (_allowNull)
        {
            out->flags = AVI_KEY_FRAME;
	    if (!_refCopy)
                out->blacken();
	    else
	        out->_noPicture = 1;
            printf ("\n[lavc] ignoring that we got no picture\n");
	    return 1;
        }
    }
    if (hurryUp)
    {
        out->flags = frameType ();
        return 1;
    }

    if(!decodeErrorHandler(ret))
        return false;

    if(!bFramePossible())
    {
        // No delay, the value is sure, no need to hide it in opaque
        _frame->reordered_opaque=(int64_t)in->demuxerPts;
    }

    // We have an image....
    bool swap = false;
    ADM_pixelFormat pix_fmt;
    pix_fmt = admPixFrmtFromLav(_context->pix_fmt, &swap);
    if (pix_fmt == ADM_PIXFRMT_INVALID)
    {
        printf ("[lavc] Unhandled pixel format: %d (AV_PIX_FMT_YUV444P10BE=%d)\n", _context->pix_fmt,AV_PIX_FMT_YUV444P10BE);
        return 0;
    }
    out->_pixfrmt = pix_fmt;

    // make sure the output is not marked as a hw image
    int count = 0;
    while(out->refType != ADM_HW_NONE && count < 32 /* arbitrary limit */)
    {
        out->hwDecRefCount();
        count++;
    }
    clonePic(_frame, out, swap);
    //printf("[AvCodec] Pts : %"PRIu64" Out Pts:%"PRIu64" \n",_frame.pts,out->Pts);

  return 1;
}
// *******************************************************************
// *******************************************************************
// *******************************************************************

decoderFFDiv3::decoderFFDiv3 (uint32_t w, uint32_t h,uint32_t fcc, uint32_t extraDataLen, uint8_t *extraData,uint32_t bpp):
decoderFF (w, h,fcc,extraDataLen,extraData,bpp)
{
  _refCopy = 1;			// YUV420 only
  WRAP_Open (AV_CODEC_ID_MSMPEG4V3);
}

decoderFFMpeg4::decoderFFMpeg4 (uint32_t w, uint32_t h,uint32_t fcc, uint32_t extraDataLen, uint8_t *extraData,uint32_t bpp):
decoderFF (w, h,fcc,extraDataLen,extraData,bpp)
{
// force low delay as avidemux don't handle B-frames
  ADM_info ("[lavc] Using %d bytes of extradata for MPEG4 decoder\n", (int)extraDataLen);

  _refCopy = 1;			// YUV420 only
  _setFcc=true;
  decoderMultiThread ();
  if(_usingMT && _threads > 2)
  {
        ADM_warning("%u threads requested, reducing to 2\n",_threads);
        _threads=2; // else we cannot handle placeholder frames following a keyframe. FIXME
  }
  WRAP_Open (AV_CODEC_ID_MPEG4);
}
bool decoderFFMpeg4::uncompress(ADMCompressedImage *in, ADMImage *out)
{
    // For pseudo startcode
    if(!_drain && in->dataLength && in->dataLength < ADM_COMPRESSED_MAX_DATA_LENGTH - 2)
        memset(in->data+in->dataLength,0,2);
    return decoderFF::uncompress(in,out);
}
//************************************
decoderFFDV::decoderFFDV (uint32_t w, uint32_t h,uint32_t fcc, uint32_t extraDataLen, uint8_t *extraData,uint32_t bpp):
decoderFF (w, h,fcc,extraDataLen,extraData,bpp)
{
    
    WRAP_Open (AV_CODEC_ID_DVVIDEO);
  

}
//************************************
decoderFFficv::decoderFFficv(uint32_t w, uint32_t h, uint32_t fcc, uint32_t extraDataLen, uint8_t *extraData, uint32_t bpp) : decoderFF(w,h,fcc,extraDataLen,extraData,bpp)
{
    WRAP_Open (AV_CODEC_ID_FIC)
}
/**
    \fn uncompress
*/
bool decoderFFficv::uncompress(ADMCompressedImage *in, ADMImage *out)
{
    if(decoderFF::uncompress(in,out))
    {
        if(in->flags&AVI_KEY_FRAME)
            out->flags=AVI_KEY_FRAME;
        return true;
    }
    return false;
}
//************************************
decoderFFMpg1::decoderFFMpg1(uint32_t w, uint32_t h, uint32_t fcc, uint32_t extraDataLen, uint8_t *extraData, uint32_t bpp):
                decoderFF(w,h,fcc,extraDataLen,extraData,bpp)
{
    _refCopy = 1;
    decoderMultiThread();
    WRAP_Open(AV_CODEC_ID_MPEG1VIDEO)
}

decoderFFMpeg12::decoderFFMpeg12 (uint32_t w, uint32_t h,uint32_t fcc, uint32_t extraDataLen, uint8_t *extraData,uint32_t bpp):
                decoderFF (w, h,fcc,extraDataLen,extraData,bpp)
{
  _refCopy = 1;			// YUV420 only
  decoderMultiThread ();
  WRAP_Open (AV_CODEC_ID_MPEG2VIDEO);
}

decoderFFPng::decoderFFPng(uint32_t w, uint32_t h, uint32_t fcc, uint32_t extraDataLen, uint8_t *extraData, uint32_t bpp) : decoderFF(w, h, fcc, extraDataLen, extraData, bpp)
{
	WRAP_Open (AV_CODEC_ID_PNG);
}

decoderFF_ffhuff::decoderFF_ffhuff (uint32_t w, uint32_t h,uint32_t fcc, uint32_t extraDataLen, uint8_t *extraData,uint32_t bpp)
:decoderFF (w, h,fcc,extraDataLen,extraData,bpp)
{
    
  _setBpp=true;
  ADM_info ("[lavc] FFhuff: We have %d bytes of extra data\n", (int)extraDataLen);
  WRAP_Open (AV_CODEC_ID_FFVHUFF);  

}
decoderFFH264::decoderFFH264 (uint32_t w, uint32_t h,uint32_t fcc, uint32_t extraDataLen, uint8_t *extraData,uint32_t bpp)
        :decoderFF (w, h,fcc,extraDataLen,extraData,bpp)
{
  _refCopy = 1;			// YUV420 only
  decoderMultiThread ();
  ADM_info ("[lavc] Initializing H264 decoder with %d extradata\n", (int)extraDataLen);
  WRAP_Open(AV_CODEC_ID_H264);
#ifdef ADM_DEBUG
  //_context->debug |= FF_DEBUG_MMCO;
  //_context->debug |= FF_DEBUG_PICT_INFO;
#endif
}
//*********************
extern "C" {int av_getAVCStreamInfo(AVCodecContext *avctx, uint32_t  *nalSize, uint32_t *isAvc);}
/**
    \fn uncompress
*/
bool   decoderFFH264::uncompress (ADMCompressedImage * in, ADMImage * out)
{
  if(!hurryUp) return decoderFF::uncompress (in, out);
    ADM_assert(0);
#if 0
  uint32_t nalSize, isAvc;
  av_getAVCStreamInfo(_context,&nalSize,&isAvc);
  if(isAvc)
  {
      return extractH264FrameType(nalSize, in->data,in->dataLength,&(out->flags));
  }else
  {
    return extractH264FrameType_startCode(nalSize, in->data,in->dataLength,&(out->flags));
  }
#endif
  return true;
}
decoderFFH265::decoderFFH265 (uint32_t w, uint32_t h,uint32_t fcc, uint32_t extraDataLen, uint8_t *extraData,uint32_t bpp)
        :decoderFF (w, h,fcc,extraDataLen,extraData,bpp)
{
  _refCopy = 1;			// YUV420 only
  decoderMultiThread ();
  ADM_info ("[lavc] Initializing H265 decoder with %d extradata\n", (int)extraDataLen);
  WRAP_Open(AV_CODEC_ID_HEVC);
  
}
//*********************
decoderFFhuff::decoderFFhuff (uint32_t w, uint32_t h,uint32_t fcc, uint32_t extraDataLen, uint8_t *extraData,uint32_t bpp):
decoderFF (w, h,fcc,extraDataLen,extraData,bpp)
{
  _setBpp=true;
  WRAP_Open (AV_CODEC_ID_HUFFYUV);
}

//***************
extern void     avcodec_init(void );
extern  void    avcodec_register_all(void );
extern "C"
{
  void adm_lavLogCallback(void  *instance, int level, const char* fmt, va_list list);
}


extern "C" 
{
static  void ffFatalError(const char *what,int lineno, const char *filez)
{
        ADM_backTrack(what,lineno,filez);
}

}
/**
    \fn ADM_lavInit
    \brief Init both lavcodec and lavformat
*/
void ADM_lavInit(void)
{
    avcodec_register_all();
    av_log_set_callback(adm_lavLogCallback);
    av_setFatalHandler(ffFatalError);
    av_log_set_level(LAV_VERBOSITY_LEVEL);
}
void adm_lavLogCallback(void  *instance, int level, const char* fmt, va_list list)
{
   // if(level>1) return;
    char buf[256];

    vsnprintf(buf, sizeof(buf), fmt, list);
    if(buf[0] != '\0' && level<=LAV_VERBOSITY_LEVEL)
        ADM_info("[lavc] %s",buf);
}

void ADM_lavDestroy(void)
{
	//av_free_static();
}

// EOF
