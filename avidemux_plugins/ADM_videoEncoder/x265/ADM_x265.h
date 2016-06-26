/***************************************************************************
                          \fn x265Encoder
                          \brief Internal handling of video encoders
                             -------------------
    
    copyright            : (C) 2002/2014 by mean/gruntster
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
#ifndef ADM_x265_H
#define ADM_x265_H
#include "ADM_coreVideoEncoder.h"
#include "ADM_encoderConf.h"
#include "x265_settings.h"
extern "C"
{
#include "x265.h"
};

#define X265_DEFAULT_CONF \
{ \
   true, /* bool UseAdvancedConfiguration */ \
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
    99, /* Pool Threads : auto */ \
    99, /* Frame Threads : auto */ \
    std::string(""), /* Preset */ \
    std::string(""), /* Tuning */ \
    std::string(""), /* Profile */ \
    }, \
    -1, /* Level */ \
    {1,1}, /* Sar width/height */ \
    3, /* uint32_t MaxRefFrames */ \
    25, /* uint32_t MinIdr */ \
    250, /* uint32_t MaxIdr */ \
    40, /* uint32_t i_scenecut_threshold */ \
    3, /* uint32_t MaxBFrame */ \
    1, /* uint32_t i_bframe_adaptative */ \
    0, /* uint32_t i_bframe_bias */ \
    2, /* uint32_t i_bframe_pyramid */ \
    1, /* bool b_deblocking_filter */ \
    0, /* uint32_t interlaced_mode */ \
    false, /* bool constrained_intra */ \
    40,	/* uint32_t lookahead; */ \
    2, /*    uint32_t weighted_pred */ \
    1, /*    bool weighted_bipred */ \
    0, /*    uint32_t cb_chroma_offset */ \
    0, /*    uint32_t cr_chroma_offset */ \
    3, /*    uint32_t me_method */ \
    16, /*   uint32_t me_range */ \
    5, /*    uint32_t subpel_refine */ \
    1, /*    uint32_t trellis */ \
    1.0, /*     float psy_rd */ \
    true, /*    bool fast_pskip */ \
    true, /*    bool dct_decimate */ \
    0, /*    uint32_t noise_reduction */ \
    0, /*    uint32_t noise_reduction_intra */ \
    0, /*    uint32_t noise_reduction_inter */ \
	{ /* Rate Control */ \
	0,	/* uint32_t rc_method; */ \
	0,	/* uint32_t qp_constant; */ \
	4,	/* uint32_t qp_step; */ \
	0,	/* uint32_t bitrate; */ \
	1.0,	/* float rate_tolerance; */ \
	0,	/* uint32_t vbv_max_bitrate; */ \
	0,	/* uint32_t vbv_buffer_size; */ \
	1,	/* uint32_t vbv_buffer_init; */ \
	1.4,	/* float ip_factor; */ \
	1.3,	/* float pb_factor; */ \
	2,	/* uint32_t aq_mode; */ \
	1.0,	/* float aq_strength; */ \
	true, 	/* bool cu_tree; */ \
        false   /* bool strict_cbr; */ \
	} \
}

/**
        \class x265Encoder
        \brief x265 HEVC encoder

*/
class x265Encoder : public ADM_coreVideoEncoder
{
protected:
               x265_param      param;
               x265_encoder    *handle;
               x265_picture    pic;
               int             plane;
               bool            globalHeader;
               bool            preAmble (ADMImage * in);
               bool            postAmble (ADMBitstream * out,uint32_t nbNals,x265_nal *nal,x265_picture *picout);
               bool            createHeader(void);
               int             encodeNals(uint8_t *buf, int size, x265_nal *nals, int nalCount, bool skipSei);
               uint32_t        extraDataLen;
               uint8_t         *extraData;
               uint32_t        seiUserDataLen;
               uint8_t         *seiUserData ;
               bool            firstIdr;
               uint32_t        passNumber;
               char            *logFile;

               
public:

                           x265Encoder(ADM_coreVideoFilter *src,bool globalHeader);
virtual                    ~x265Encoder();
virtual        bool        setup(void); 
virtual        bool        encode (ADMBitstream * out);
virtual const  char        *getFourcc(void) {return "HEVC";}
virtual        bool         getExtraData(uint32_t *l,uint8_t **d) {*l=extraDataLen;*d=extraData;return true;}
virtual        bool         isDualPass(void) ;

virtual        bool         setPassAndLogFile(int pass,const char *name);


};

#endif
