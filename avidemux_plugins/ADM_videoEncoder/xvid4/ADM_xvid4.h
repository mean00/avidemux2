/***************************************************************************
                          \fn xvid4Encoder
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
#ifndef ADM_xvid4_H
#define ADM_xvid4_H
#include "ADM_coreVideoEncoder.h"
#include "ADM_encoderConf.h"
#include "xvid4_encoder.h"
#include "xvid.h"

#define XVID_DEFAULT_CONF \
{ \
    { \
    COMPRESS_CQ, /*COMPRESSION_MODE  mode */ \
    2,              /* uint32_t          qz           /// Quantizer */ \
    1500,           /*uint32_t          bitrate      /// In kb/s */ \
    700,            /*uint32_t          finalsize    /// In ? */ \
    1500,           /*uint32_t          avg_bitrate  /// avg_bitrate is in kb/s!! */ \
    ADM_ENC_CAP_CBR+ADM_ENC_CAP_CQ+ADM_ENC_CAP_2PASS+ADM_ENC_CAP_2PASS_BR+ADM_ENC_CAP_GLOBAL+ADM_ENC_CAP_SAME \
    }, \
    XVID_PROFILE_AS_L4, /* Profile */ \
    3, /* rdMode */ \
    3, /* MotionEstimation */ \
    0, /* cqmMode */ \
    1, /* arMode */ \
    2, /* MaxBframe */ \
    200, /* MaxKeyInterval */ \
    99, /* nbThreads */ \
    2, /* Qmin */\
    25, /* Qmax */\
    true, /* rdOnBframe */ \
    true, /*bool:hqAcPred */ \
    true, /*bool:optimizeChrome */ \
    true, /* Trellis */ \
    false, /* UseXvidFCC */ \
}

/**
        \class xvid4Encoder
        \brief Xvid4 mpeg4 encoder

*/
class xvid4Encoder : public ADM_coreVideoEncoder
{
protected:
               
               void           *handle;
               int             plane;
               bool            globalHeader;
               bool            preAmble (ADMImage * in);
               bool            postAmble (ADMBitstream * out,xvid_enc_stats_t *stat,int size);
               bool            query(void);

               bool            setupPass(void);

                xvid_plugin_single_t single;
                xvid_plugin_2pass1_t pass1;
                xvid_plugin_2pass2_t pass2;
                xvid_enc_frame_t xvid_enc_frame;
                xvid_enc_stats_t xvid_enc_stats;
                xvid_enc_plugin_t plugins[7];

                uint32_t        frameNum;
                uint32_t        currentRef;
                uint32_t        backRef;
                uint32_t        fwdRef;
                uint32_t        refIndex;

                std::string     logFile;
                int             pass;
public:

                           xvid4Encoder(ADM_coreVideoFilter *src,bool globalHeader);
virtual                    ~xvid4Encoder();
virtual        bool        setup(void); 
virtual        bool        encode (ADMBitstream * out);
virtual const  char        *getFourcc(void);

virtual        bool        isDualPass(void) ;
static         int         hook (void *handle, int opt, void *param1, void *param2);
               bool        setPassAndLogFile(int pass,const char *name);
};

#endif
