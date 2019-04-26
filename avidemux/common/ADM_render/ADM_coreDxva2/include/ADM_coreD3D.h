/***************************************************************************
    \file             : ADM_coreDxva2.cpp
    \brief            : Wrapper around dxva functions
    \author           : (C) 2016 by mean fixounet@free.fr
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

#include "ADM_coreDxva2_export.h"
#include "ADM_image.h"
#include "ADM_windowInfo.h"

#ifdef _WIN32_WINNT
  #undef _WIN32_WINNT
#endif
#define _WIN32_WINNT 0x0600
#define DXVA2API_USE_BITFIELDS
#include <d3d9.h>

/**
    \class admVdpau
*/
class ADM_COREVIDEOCODEC6_EXPORT admD3D
{
public:
    enum ADM_vendorID
    {
      VENDOR_UNKNOWN=0,
      VENDOR_AMD=1,
      VENDOR_NVIDIA=2,
      VENDOR_INTEL=3
    };
public:
        static bool init(GUI_WindowInfo *x);
        static bool isOperationnal(void);
        static bool cleanup(void);
        static IDirect3DDevice9 *getDevice();
        static IDirect3D9       *getHandle();
        static bool              isDirect9Ex();
        static ADM_vendorID      getVendorID();
        static int64_t           getDriverVersion();
};
