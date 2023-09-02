/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef ADM_AV1AOM_H
#define ADM_AV1AOM_H
#include "ADM_coreVideoEncoder.h"
#include "ADM_encoderConf.h"

#include "aom/aom.h"
#include "aom/aomcx.h"
#include "aom/aom_encoder.h"

#define AV1_ENC_MAX_THREADS 32

#define AV1_DEFAULT_CONF \
{ \
    { \
        COMPRESS_CQ, /* COMPRESSION_MODE mode */ \
        20, /* qz */ \
        2000, /* bitrate in kb/s */ \
        200, /* finalsize in MiB */ \
        0, /* avg_bitrate in kb/s */ \
        ADM_ENC_CAP_CBR+ADM_ENC_CAP_CQ+ADM_ENC_CAP_2PASS+ADM_ENC_CAP_2PASS_BR+ADM_ENC_CAP_GLOBAL \
    }, \
     2, /* nbThreads */ \
     1, /* autoThreads */ \
     0, /* usage */ \
     4, /* speed */ \
     0, /* tiling */ \
   128, /* keyint */ \
     0  /* fullrange */ \
}

/**
    \class av1AomEncoder
    \brief libaom AV1 encoder
*/
class av1AomEncoder : public ADM_coreVideoEncoder
{
protected:
                aom_codec_ctx_t     context;
                aom_codec_enc_cfg_t param;
                aom_codec_iface_t   *iface;
                aom_image_t         *pic;
                std::vector <ADMBitstream *> outQueue;

                int                 plane;
                uint32_t            scaledFrameDuration;
                bool                flush;

                bool                globalStreamHeader;
                uint8_t             *extraData;
                uint32_t            extraDataLen;

                std::string         logFile;
                FILE                *statFd;
                int                 passNumber;
                void                *statBuf;
                uint64_t            lastDts;
                uint64_t            lastScaledPts; // in timebase units!

                bool                postAmble(ADMBitstream *out);
public:
                                    av1AomEncoder(ADM_coreVideoFilter *src, bool globalHeader);
    virtual                         ~av1AomEncoder();
    virtual     bool                setup(void);
    virtual     bool                getExtraData(uint32_t *l, uint8_t **d) { *l = extraDataLen; *d = extraData; return true; }
    virtual     bool                encode(ADMBitstream *out);
    virtual     const char          *getFourcc(void) { return "av01"; }
    virtual     bool                isDualPass(void);
    virtual     bool                setPassAndLogFile(int pass, const char *name);
};

#endif
