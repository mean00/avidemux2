/***************************************************************************
    \file             : ADM_coreVideoToolbox.h
    \brief            : Wrapper around VideoToolbox functions
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef ADM_CORE_VIDEOTOOLBOX_H
#define ADM_CORE_VIDEOTOOLBOX_H

#include "ADM_windowInfo.h"

class admCoreVideoToolbox
{
public:
    static bool         init(GUI_WindowInfo *x) { return true; }
    static bool         isOperationnal(void) { return true; }
    static bool         cleanup(void) {return true; }

    static int          initVideoToolbox(AVCodecContext *avctx);
    static int          copyData(AVCodecContext *avctx, AVFrame *src, ADMImage *dest);
    static void         uninit(AVCodecContext *avctx);
};

#endif