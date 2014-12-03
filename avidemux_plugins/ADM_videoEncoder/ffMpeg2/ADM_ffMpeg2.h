/***************************************************************************
                          \fn ADM_VideoEncoders
                          \brief Internal handling of video encoders
                             -------------------
    
    copyright            : (C) 2002/2009 by mean
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
#ifndef ADM_ffMpeg2_ENCODER_H
#define ADM_ffMpeg2_ENCODER_H
#include "ADM_coreVideoEncoderFFmpeg.h"
#include "mpeg2_encoder.h"

enum
{
        MPEG2_MATRIX_DEFAULT,
        MPEG2_MATRIX_TMPGENC,
        MPEG2_MATRIX_ANIME,
        MPEG2_MATRIX_KVCD,
        MPEG2_MATRIX_LAST
} ;

#define MPEG2_CONF_DEFAULT \
{ \
    { \
    COMPRESS_CQ, /* COMPRESSION_MODE  mode */ \
    2,              /* uint32_t          qz           /// Quantizer */ \
    1500,           /* uint32_t          bitrate      /// In kb/s */ \
    700,            /* uint32_t          finalsize    /// In ? */ \
    1500,           /* uint32_t          avg_bitrate  /// avg_bitrate is in kb/s!! */ \
    ADM_ENC_CAP_CBR+ADM_ENC_CAP_CQ+ADM_ENC_CAP_2PASS+ADM_ENC_CAP_2PASS_BR+ADM_ENC_CAP_GLOBAL+ADM_ENC_CAP_SAME \
    }, \
    { \
        ADM_AVCODEC_SETTING_VERSION, \
        2, /* Multithreaded */ \
          ME_EPZS,			/* ME */ \
          0,				/* GMC */ \
          0,				/* 4MV */ \
          0,				/* _QPEL */ \
          1,				/* _TREILLIS_QUANT */ \
          2,				/* qmin */ \
          31,				/* qmax */ \
          3,				/* max_qdiff */ \
          2,				/* max_b_frames */ \
          1,				/* mpeg_quant */ \
          1,				/* is_luma_elim_threshold */ \
          -2,				/* luma_elim_threshold */ \
          1,				/* is_chroma_elim_threshold */ \
          -5,				/* chroma_elim_threshold */ \
          0.05,				/*lumi_masking */ \
          1,				/* is lumi */ \
          0.01,				/*dark_masking */ \
          1,				/* is dark */ \
          0.5,				/* qcompress amount of qscale change between easy & hard scenes (0.0-1.0 */ \
          0.5,				/* qblur    amount of qscale smoothing over time (0.0-1.0) */ \
          0,   		        /* min bitrate in kB/S */ \
          9500,  			/* max bitrate */ \
          1,				/* user matrix */ \
          18,				/* gop size */ \
          0,				/* interlaced */ \
          0,				/* WLA: bottom-field-first */ \
          0,				/* wide screen */ \
          2,				/* mb eval = distortion */ \
          8000,				/* vratetol 8Meg */ \
          0,				/* is temporal */ \
          0.0,				/* temporal masking */ \
          0,				/* is spatial */ \
          0.0,				/* spatial masking */ \
          0,				/* Normalize aqp */ \
          0,                /* Xvid rate control */ \
          224,              /* Buffer size (kbits) */ \
          0,                /* Override ratecontrol */ \
          0,    		    /* DUMMY */ \
    }, \
    0   /* Matrix */ \
}

/**
        \class ADM_ffMpeg2Encoder
        \brief Dummy encoder that does nothing

*/
class ADM_ffMpeg2Encoder : public ADM_coreVideoEncoderFFmpeg
{
protected:
               int             plane;
               mpeg2_encoder   settings;
public:

                           ADM_ffMpeg2Encoder(ADM_coreVideoFilter *src,bool globalHeader);
virtual                    ~ADM_ffMpeg2Encoder();
virtual        bool        configureContext(void);
virtual        bool        setup(void); 
virtual        bool        encode (ADMBitstream * out);
virtual const  char        *getFourcc(void) {return "MPEG";}

virtual        bool         isDualPass(void) ;

};

#endif
