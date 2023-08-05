/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#pragma once
#include "ADM_hwAccel.h"
#include "BVector.h"

#ifdef USE_NVENC
#define FRAMES_POOL_SIZE 20

typedef struct ADM_nvDecRef {
    AVFrame *pic;
    int refCount;
} ADM_nvDecRef;

/**
 *  \class decoderFFnvDec
 */
class decoderFFnvDec : public ADM_acceleratedDecoderFF
{
protected:

                    admMutex    *renderMutex;
                    BVector <ADM_nvDecRef *> pool;
                    ADMColorScalerSimple *color8bits;
                    ADMColorScalerSimple *color10bits;
                    bool        alive;

                    bool        readBackBuffer(AVFrame *decodedFrame, ADMCompressedImage *in, ADMImage *out);

public:
                                decoderFFnvDec(AVCodecContext *avctx, decoderFF *parent);
                                ~decoderFFnvDec();

                    bool        markRefUsed(ADM_nvDecRef *render);
                    bool        markRefUnused(ADM_nvDecRef *render);
                    bool        downloadFromRef(ADM_nvDecRef *render, ADMImage *image);

    virtual         bool        uncompress (ADMCompressedImage * in, ADMImage * out);
    virtual const   char        *getName(void) {return "NVDEC";}
    virtual         bool        isAlive(void) {return alive;}
};
#endif
//EOF
