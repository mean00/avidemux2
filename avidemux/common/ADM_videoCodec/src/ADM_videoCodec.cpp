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

#include "ADM_default.h"
#include "ADM_codec.h"
#include "ADM_ffmp43.h"
#include "fourcc.h"
#include "ADM_codecVdpau.h"
#include "ADM_codecXvba.h"
#include "ADM_codecLibVA.h"
#include "DIA_coreToolkit.h"

#if defined(USE_VPX)
#include "ADM_vpx.h"
#endif


extern bool vdpauUsable(void);
extern bool xvbaUsable(void);
extern bool libvaUsable(void);

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
  

#if defined(USE_XVBA) 
  ADM_info("Searching decoder in xvba (%d x %d, extradataSize:%d)...\n",w,h,extraLen);
  if (isH264Compatible (fcc) )
    {
        ADM_info("This is xvba compatible\n");
        if(true==xvbaUsable())
        {
            decoderFFXVBA *dec=new decoderFFXVBA (w,h,fcc,extraLen,extraData,bpp);
            if(dec->initializedOk()==true)
                return (decoders *) (dec);
            else
            {
                GUI_Error_HIG("XVBA","Cannot initialize XVBA, make sure it is not already used by another application.\nSwitching to default decoder.");
                delete dec;
            }
        }else ADM_info("XVBA is not active\n");
    }        
#endif // XVBA  
#if defined(USE_LIBVA) 
  ADM_info("Searching decoder in libva (%d x %d, extradataSize:%d)...\n",w,h,extraLen);
  if(decoderFFLIBVA::fccSupported(fcc))
  {
        ADM_info("This is libva compatible\n");
        if(true==libvaUsable())
        {
            decoderFFLIBVA *dec=new decoderFFLIBVA (w,h,fcc,extraLen,extraData,bpp);
            if(dec->initializedOk()==true)
                return (decoders *) (dec);
            else
            {
                GUI_Error_HIG("LIBVA","Cannot initialize LIBVA, make sure it is not already used by another application.\nSwitching to default decoder.");
                delete dec;
            }
        }else ADM_info("LIBVA is not active\n");
    }
#endif // XVBA  
  
  
    ADM_info("Searching decoder in coreVideoCodec(%d x %d, extradataSize:%d)...\n",w,h,extraLen);
    return ADM_coreCodecGetDecoder(fcc,w,h,extraLen,extraData,bpp);
}
//EOF

