/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef ADM_VP8_H
#define ADM_VP8_H
#include "ADM_coreVideoEncoder.h"
#include "ADM_encoderConf.h"

extern "C"
{
#include "vpx/vpx_encoder.h"
};

enum vp8DeadlinePreset
{
    REALTIME=0,
    GOOD_QUALITY=1,
    BEST_QUALITY=2
};

#define VP8_ENC_MAX_THREADS 16

#define VP8_DEFAULT_CONF \
{ \
    { \
        COMPRESS_CQ, /* COMPRESSION_MODE mode */ \
        20, /* qz */ \
        2000, /* bitrate in kb/s */ \
        200, /* finalsize in MiB */ \
        1500, /* avg_bitrate in kb/s */ \
        ADM_ENC_CAP_CBR+ADM_ENC_CAP_CQ+ADM_ENC_CAP_2PASS+ADM_ENC_CAP_2PASS_BR+ADM_ENC_CAP_GLOBAL \
    }, \
     2, /* nbThreads */ \
     1, /* autoThreads */ \
    16, /* speed = VP8E_SET_CPUUSED + 16 */ \
     1, /* deadline = 1s */ \
   128 /* keyint */ \
}

/**
    \class vp8Encoder
    \brief libvpx VP8 encoder
*/
class vp8Encoder : public ADM_coreVideoEncoder
{
protected:
                vpx_codec_ctx_t     context;
                vpx_codec_enc_cfg_t param;
                vpx_codec_iface_t   *iface;
                vpx_image_t         *pic;
                std::vector <const vpx_codec_cx_pkt *> packetQueue;

                int                 plane;
                uint32_t            scaledFrameDuration;
                uint32_t            dline;
                bool                flush;
                bool                postAmble(ADMBitstream *out);

                std::string         logFile;
                FILE                *statFd;
                int                 passNumber;
                void                *statBuf;
                uint64_t            lastDts;
                uint64_t            lastScaledPts; // in timebase units!
public:
                                    vp8Encoder(ADM_coreVideoFilter *src, bool globalHeader);
    virtual                         ~vp8Encoder();
    virtual     bool                setup(void);
    virtual     bool                encode(ADMBitstream *out);
    virtual     const char          *getFourcc(void) {return "VP8 ";}
    virtual     bool                isDualPass(void);
    virtual     bool                setPassAndLogFile(int pass, const char *name);
};

#endif
