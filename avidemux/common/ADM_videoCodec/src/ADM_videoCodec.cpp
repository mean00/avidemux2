/***************************************************************************
        \file ADM_videoCodec
        \brief Search and instantiate video coder
        \author mean fixounet@free.fr (C) 2010

    see here : http://www.webartz.com/fourcc/

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

extern "C"
{
#include "ADM_lavcodec.h"
};
#include "ADM_default.h"
#include "ADM_codec.h"
#include "ADM_ffmp43.h"
#include "avidemutils.h"
#include "fourcc.h"
#include "ADM_codecVdpau.h"
#include "DIA_coreToolkit.h"

#if defined(USE_VPX)
#include "ADM_vpx.h"
#endif


extern bool vdpauUsable(void);
decoders *tryCreatingVideoDecoder(uint32_t w, uint32_t h, uint32_t fcc,uint32_t extraDataLen,
                    uint8_t *extra, uint32_t bpp);
/**
    \fn getDecoder
    \brief returns the correct decoder for a stream w,h,fcc,extraLen,extraData,bpp
*/
decoders *ADM_getDecoder (uint32_t fcc, uint32_t w, uint32_t h, uint32_t extraLen, 
            uint8_t * extraData,uint32_t bpp)
{
  ADM_info("\nSearching decoder in plugins\n");
  decoders *fromPlugin=tryCreatingVideoDecoder(w,h,fcc,extraLen,extraData,bpp);
  if(fromPlugin) return fromPlugin;
#if defined(USE_VDPAU) 
  ADM_info("Searching decoder in vdpau (%d x %d, extradataSize:%d)...\n",w,h,extraLen);
  if (isH264Compatible (fcc) || isMpeg12Compatible(fcc) || 0*isVC1Compatible(fcc))
    {
        ADM_info("This is vdpau compatible\n");
        if(true==vdpauUsable())
        {
            decoderFFVDPAU *dec=new decoderFFVDPAU (w,h,fcc,extraLen,extraData,bpp);
            if(dec->initializedOk()==true)
                return (decoders *) (dec);
            else
            {
                GUI_Error_HIG("VDPAU","Cannot initialize VDPAU, make sure it is not already used by another application.\nSwitching to default decoder.");
                delete dec;
            }
        }else ADM_info("Vdpau is not active\n");
    }        
#endif // VDPAU

// VPX
#if defined(USE_VPX)
    if(fourCC::check(fcc,(const uint8_t *)"VP8 "))
    {
        decoderVPX *dec=new decoderVPX(w,h,fcc,extraLen,extraData,bpp);
        if(dec->initializedOk())
            return (decoders *) (dec);
        GUI_Error_HIG("VPX","Cannot initialize libvpx.");
        delete dec;
    }
#endif // VPX
    ADM_info("Searching decoder in coreVideoCodec(%d x %d, extradataSize:%d)...\n",w,h,extraLen);
    return ADM_coreCodecGetDecoder(fcc,w,h,extraLen,extraData,bpp);
}
//EOF

