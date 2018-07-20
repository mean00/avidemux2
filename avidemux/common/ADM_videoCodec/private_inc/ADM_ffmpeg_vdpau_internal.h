/***************************************************************************
            \file              ADM_ffmpeg_vdpau.cpp  
            \brief Decoder using half ffmpeg/half VDPAU

    The ffmpeg part is to preformat inputs for VDPAU
    VDPAU is loaded dynamically to be able to make a binary
        and have something working even if the target machine
        does not have vdpau


 ***************************************************************************/

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
#include "ADM_coreVdpau.h"
#include <vector>
/**
 */
struct AVVDPAUContext;

typedef struct 
{
        std::vector <ADM_vdpauRenderState *>freeQueue;
        std::vector <ADM_vdpauRenderState *>fullQueue;
}vdpauContext;

/**
 *  \class decoderFFVDPAU
 */

class decoderFFVDPAU:public ADM_acceleratedDecoderFF
{
protected:
                    bool            alive;
                    vdpauContext    vdpau;
protected:
                    bool        initVdpContext();
public:     // Callbacks
                    int         getBuffer(AVCodecContext *avctx, AVFrame *pic);
                    void        releaseBuffer(struct ADM_vdpauRenderState *rdr);
                    bool        initFail(void) {alive=false;return true;}
public:
            // public API
                                decoderFFVDPAU (AVCodecContext *avctx,decoderFF *parent);
                                ~decoderFFVDPAU();
    virtual         bool        uncompress (ADMCompressedImage * in, ADMImage * out);
                    bool        readBackBuffer(AVFrame *decodedFrame, ADMCompressedImage * in, ADMImage * out);    
    virtual const   char        *getName(void)        {return "VDPAU";}
                    
};
