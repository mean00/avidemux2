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
ADM_hwAccelEntry    *ADM_hwAccelManager::lookup(struct AVCodecContext *avctx,  const enum AVPixelFormat *fmt)
{
    int n=listOfHwAccel.size();
    for( int i=0;i<n;i++)
    {
        ADM_hwAccelEntry *e=listOfHwAccel[i];
        enum AVPixelFormat out;
        if(true==e->canSupportThis(avctx,fmt,out))
        {
            ADM_info("Matching hw accel : %s\n",e->name);
            return e;
        }
    }
    ADM_info("No Matching Hw accel\n");
    return NULL;;
}
/**
 * \fn spawn
 */
ADM_acceleratedDecoder *ADM_hwAccelManager::spawn( struct AVCodecContext *avctx,  const enum AVPixelFormat *fmt,enum AVPixelFormat &outputFormat )
{
    ADM_hwAccelEntry *e=lookup(avctx,fmt);
    ADM_assert(e);
    ADM_assert(e-> canSupportThis(avctx,  fmt,outputFormat));
    return e->spawn();
    
}
/**
 * 
 * @param avctx
 * @param fmt
 * @return 
 */
extern enum AVPixelFormat ADM_FFgetFormat(struct AVCodecContext *avctx,  const enum AVPixelFormat *fmt)
{
    return avcodec_default_get_format(avctx,fmt);
}

// EOF