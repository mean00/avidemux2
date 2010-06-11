/***************************************************************************
            \file  ADM_vpx.h
            \brief I/f to libvpx (decoder)

    The ffmpeg part is to preformat inputs for VDPAU
    VDPAU is loaded dynamically to be able to make a binary
        and have something working even if the target machine
        does not have vdpau
    Some part, especially get/buffer and ip_age borrowed from xbmc
        as the api from ffmpeg is far from clear....


 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef ADM_VPX_H
#define ADM_VPX_H
#include "ADM_codec.h"
/**
    \class decoderVpx
*/
class decoderVPX:public decoders
{
protected:
                    bool alive;
                    void     *vpx;
public:
            // public API
                    decoderVPX (uint32_t w, uint32_t h,uint32_t fcc, uint32_t extraDataLen, uint8_t *extraData,uint32_t bpp);
                    ~decoderVPX();
    virtual bool    uncompress (ADMCompressedImage * in, ADMImage * out);
    virtual bool bFramePossible (void)
      {
        return 0;
      }
    virtual const char *getDecoderName(void) {return "LibVPX";}
    virtual bool  initializedOk(void)        {return alive;};
    virtual bool dontcopy (void)             {return true;}
};

#endif
//EOF

