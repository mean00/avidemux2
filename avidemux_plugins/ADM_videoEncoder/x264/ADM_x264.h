/***************************************************************************
                          \fn x264Encoder
                          \brief Internal handling of video encoders
                             -------------------
    
    copyright            : (C) 2002/2010 by mean/gruntster
    email                : fixounet@free.fr
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef ADM_x264_H
#define ADM_x264_H
#include "ADM_coreVideoEncoder.h"
#include "ADM_encoderConf.h"
#include "x264_encoder.h"
extern "C"
{
#include "x264.h"
};

#define X264_DEFAULT_CONF \
{ \
   { /* General */ \
    { \
    COMPRESS_AQ, /* COMPRESSION_MODE  mode */ \
    20,              /* uint32_t          qz           /// Quantizer */ \
    1500,           /* uint32_t          bitrate      /// In kb/s */ \
    700,            /* uint32_t          finalsize    /// In ? */ \
    1500,           /* uint32_t          avg_bitrate  /// avg_bitrate is in kb/s!! */ \
        ADM_ENC_CAP_CBR+ \
        ADM_ENC_CAP_CQ+ \
        ADM_ENC_CAP_AQ+ \
        ADM_ENC_CAP_2PASS+ \
        ADM_ENC_CAP_2PASS_BR+ \
        ADM_ENC_CAP_GLOBAL+ \
        0*ADM_ENC_CAP_SAME \
    }, \
    99, /* Threads : auto */ \
    true /* Fast first pass */ \
    }, \
    31, /* Level */ \
    {1,1}, /* Sar width/height */ \
    2, /* uint32_t MaxRefFrames */ \
    100, /* uint32_t MinIdr */ \
    500, /* uint32_t MaxIdr */ \
    2, /* uint32_t MaxBFrame */ \
    0, /* uint32_t i_bframe_adaptative */ \
    0, /* uint32_t i_bframe_bias */ \
    0, /* uint32_t i_bframe_pyramid */ \
    0, /* bool b_deblocking_filter */ \
    0, /* int32_t i_deblocking_filter_alphac0 */ \
    0, /* int32_t i_deblocking_filter_beta */ \
    true, /* bool cabac */ \
    false, /* bool interlaced */ \
    true, /*    bool b_8x8 */ \
    false, /*    bool b_i4x4 */ \
    false, /*    bool b_i8x8 */ \
    false, /*    bool b_p8x8 */ \
    false, /*    bool b_p16x16 */ \
    false, /*    bool b_b16x16 */ \
    0, /*    uint32_t weighted_pred */ \
    0, /*    bool weighted_bipred */ \
    0, /*    uint32_t direct_mv_pred */ \
    0, /*    uint32_t chroma_offset */ \
    0, /*    uint32_t me_method */ \
    7, /*    uint32_t subpel_refine */ \
    false, /*    bool chroma_me */ \
    false, /*    bool mixed_references */ \
    1, /*    uint32_t trellis */ \
    true, /*    bool fast_pskip */ \
    false, /*    bool dct_decimate */ \
    0, /*    uint32_t noise_reduction */ \
    true, /*    bool psy */ \
}

/**
        \class x264Encoder
        \brief x264 mpeg4 encoder

*/
class x264Encoder : public ADM_coreVideoEncoder
{
protected:
               x264_param_t     param;
               x264_t          *handle;
               x264_picture_t  pic;
               int             plane;
               bool            globalHeader;
               bool            preAmble (ADMImage * in);
               bool            postAmble (ADMBitstream * out,uint32_t nbNals,x264_nal_t *nal,x264_picture_t *picout);
               bool            createHeader(void);
               int             encodeNals(uint8_t *buf, int size, x264_nal_t *nals, int nalCount, bool skipSei);
               uint32_t        extraDataLen;
               uint8_t         *extraData;
               uint32_t        seiUserDataLen;
               uint8_t         *seiUserData ;
               bool            firstIdr;
               uint32_t        passNumber;
               char            *logFile;

               
public:

                           x264Encoder(ADM_coreVideoFilter *src,bool globalHeader);
virtual                    ~x264Encoder();
virtual        bool        setup(void); 
virtual        bool        encode (ADMBitstream * out);
virtual const  char        *getFourcc(void) {return "H264";}
virtual        bool         getExtraData(uint32_t *l,uint8_t **d) {*l=extraDataLen;*d=extraData;return true;}
virtual        bool         isDualPass(void) ;

virtual        bool         setPassAndLogFile(int pass,const char *name);


};

#endif
