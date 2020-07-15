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

/**
 *  \class decoderFFVT
 */

class decoderFFVT : public ADM_acceleratedDecoderFF
{
protected:
                    ADMImage    *copy;
public:
                    bool        alive;
                                decoderFFVT (AVCodecContext *avctx,decoderFF *parent);
                                ~decoderFFVT();
    virtual         bool        uncompress (ADMCompressedImage * in, ADMImage * out);
    virtual const   char        *getName(void) {return "VideoToolbox";}
                    //bool        markSurfaceUsed(ADM_VTSurface *s);
                    //bool        markSurfaceUnused(ADM_VTSurface *s);
};
#endif
//EOF
