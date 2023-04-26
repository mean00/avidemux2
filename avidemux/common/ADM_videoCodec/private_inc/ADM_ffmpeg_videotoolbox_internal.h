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

#ifdef USE_VIDEOTOOLBOX
#define NB_SW_FRAMES 16
/**
 *  \class decoderFFVT
 */

class decoderFFVT : public ADM_acceleratedDecoderFF
{
protected:
                    AVFrame     *swframes[NB_SW_FRAMES];
                    int         swframeIdx;
                    bool        alive;
public:
                                decoderFFVT (AVCodecContext *avctx,decoderFF *parent);
                                ~decoderFFVT();
    virtual         bool        uncompress (ADMCompressedImage * in, ADMImage * out);
    virtual const   char        *getName(void) {return "VideoToolbox";}
    virtual         bool        isAlive(void) {return alive;}
};
#endif
//EOF
