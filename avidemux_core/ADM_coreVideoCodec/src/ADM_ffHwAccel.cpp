/***************************************************************************
    \file ADM_ffHwAccel
    \brief Decoders using lavcodec
    \author mean & all (c) 2002-2010
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <stddef.h>

#include "ADM_default.h"
#include "ADM_ffmp43.h"
#include "DIA_coreToolkit.h"
#include "../ADM_hwAccel/include/ADM_hwAccel.h"
#include <vector>

static std::vector<ADM_hwAccelEntry *>listOfHwAccel;

/**
 * \fn registerDecoder
 */
bool                ADM_hwAccelManager::registerDecoder(ADM_hwAccelEntry *entry)
{
    listOfHwAccel.push_back(entry);
    return false;
}
/**
 * \fn lookup
 */
ADM_hwAccelEntry    *ADM_hwAccelManager::lookup(struct AVCodecContext *avctx,  const enum AVPixelFormat *fmt,enum AVPixelFormat &outputFormat)
{
    int n=listOfHwAccel.size();
    for( int i=0;i<n;i++)
    {
        ADM_hwAccelEntry *e=listOfHwAccel[i];
        if(true==e->canSupportThis(avctx,fmt,outputFormat))
        {
            ADM_info("Matching hw accel : %s\n",e->name);
            return e;
        }
    }
    ADM_info("No Matching Hw accel\n");
    return NULL;;
}

/**
 * 
 * @param avctx
 * @param fmt
 * @return 
 */
extern enum AVPixelFormat ADM_FFgetFormat(struct AVCodecContext *avctx,  const enum AVPixelFormat *fmt)
{
    enum AVPixelFormat outFmt;
    ADM_hwAccelEntry    *accel=ADM_hwAccelManager::lookup(avctx,fmt,outFmt);
    if(accel)
    {
        decoderFF *ff=(decoderFF *)avctx->opaque;
        if(ff->getHwDecoder())
        {
            ADM_info("Reusing existing setup\n");        
            return outFmt;
        }else
            if(ff->setHwDecoder(accel->spawn(avctx,fmt)))
            {
                ADM_info("Using %s as hw accel\n",accel->name);
                return outFmt;
            }
    }
    ADM_info("No Hw Accel for that\n");
    return avcodec_default_get_format(avctx,fmt);
}
// EOF