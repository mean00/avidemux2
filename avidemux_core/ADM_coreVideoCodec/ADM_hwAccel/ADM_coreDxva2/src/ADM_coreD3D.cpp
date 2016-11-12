/***************************************************************************
    \file             : ADM_coreDXVA2.cpp
    \brief            : Wrapper around DXVA2 functions
    \author           : (C) 2016 by mean fixounet@free.fr

        This code is strongly derived from ffmpeg_dxva2 from ffmpeg

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

#ifdef USE_DXVA2

#if 1
#define aprintf printf
#define DUMP_GUID
#else
#define aprintf(...) {}
#endif



#include "ADM_dynamicLoading.h"
#include <map>

#define CINTERFACE
#ifdef _WIN32_WINNT
#undef _WIN32_WINNT
#endif
#define _WIN32_WINNT 0x0600
#define DXVA2API_USE_BITFIELDS
#define COBJMACROS
#include <d3d9.h>
#include <dxva2api.h>
#include "../include/ADM_coreD3D.h"

typedef IDirect3D9* WINAPI pDirect3DCreate9(UINT);

static bool                  coreD3DWorking=false;

static ADM_LibWrapper        d3dDynaLoader;

static pDirect3DCreate9      *createD3D = NULL;

static IDirect3D9            *d3d9        =NULL;
static IDirect3DDevice9      *d3d9device  =NULL;


#include <initguid.h>
/**
 */

/**
 */
static bool ADM_FAILED(HRESULT hr)
{
    int r=(int)hr;
    if(r<0)
    {
        ADM_warning("Failed with error code=0x%x\n",r);
        return true;
    }
    return false;
}

/**
 * \fn init
 * \brief initialize the low level common part of dxva2
 */
bool admD3D::init(GUI_WindowInfo *x)
{
    UINT adapter = D3DADAPTER_DEFAULT;
    D3DDISPLAYMODE        d3ddm;
    D3DPRESENT_PARAMETERS d3dpp = {0};
    HWND windowID=(HWND)x->systemWindowId;

    HRESULT hr;
    unsigned int resetToken = 0;
    ADM_info("Probing for D3D support\n");
    // Load d3d9 dll
    if(false==d3dDynaLoader.loadLibrary("d3d9.dll"))
    {
        ADM_warning("Dxva2: Cannot load d3d9.dll\n"); // memleak
        goto failInit;
    }
    // Load d3d9 dll
    createD3D=(pDirect3DCreate9 *)d3dDynaLoader.getSymbol("Direct3DCreate9");
    if(!createD3D)
    {
        ADM_warning("Dxva2: Cannot get symbol  Direct3DCreate9\n");
        goto failInit;
    }
    d3d9 = createD3D(D3D_SDK_VERSION);
    if(!d3d9)
    {
        ADM_warning("Dxva2: Cannot gcreate d3d9\n");
        goto failInit;
    }
    ADM_info("D3D library loaded, creating instance\n");
    IDirect3D9_GetAdapterDisplayMode(d3d9, adapter, &d3ddm);
    d3dpp.Windowed         = TRUE;
    d3dpp.BackBufferWidth  = 640;
    d3dpp.BackBufferHeight = 480;
    d3dpp.BackBufferCount  = 0;
    d3dpp.BackBufferFormat = d3ddm.Format;
    d3dpp.SwapEffect       = D3DSWAPEFFECT_DISCARD;
    d3dpp.Flags            = D3DPRESENTFLAG_VIDEO;

    hr = IDirect3D9_CreateDevice(d3d9,
                                adapter,
                                D3DDEVTYPE_HAL,
                                windowID,
                                 D3DCREATE_SOFTWARE_VERTEXPROCESSING | D3DCREATE_MULTITHREADED | D3DCREATE_FPU_PRESERVE,
                                 &d3dpp, &d3d9device);
    if(ADM_FAILED(hr))
    {
        ADM_warning("D3D:Cannot create d3d9 device\n");
        goto failInit;
    }

    coreD3DWorking=true;
    ADM_info("D3D:d3d core successfully initialized\n");

    return true;
failInit:
      ADM_warning("D3D:D3D init failed\n");
      return false;
}


/**
    \fn isOperationnal
*/
bool admD3D::isOperationnal(void)
{
    ADM_warning("This binary has no working DXVA2 support\n");
    return coreD3DWorking;
}
/**
 */
bool admD3D::cleanup(void)
{
    ADM_warning("This binary has no DXVA2 support\n");

    if (d3d9device)
    {
        IDirect3DDevice9_Release(d3d9device);
        d3d9device  =NULL;
    }

    if (d3d9)
    {
        IDirect3D9_Release(d3d9);
        d3d9        =NULL;
    }
    coreD3DWorking=false;

    // small memleak  d3dDynaLoader;
    // small memleak dxva2DynaLoader;
    return true;
}
/**

*/
IDirect3DDevice9 *admD3D::getDevice()
{

        return d3d9device;
}

/**

*/
IDirect3D9       *admD3D::getHandle()
{
        return d3d9;
}


/**
 * \fn admLibVa_exitCleanup
 */
bool admD3d_exitCleanup()
{
    ADM_info("D3D cleanup begin\n");
    admD3D::cleanup();
    ADM_info("admD3D cleanup end\n");
    return true;
}

#endif
// EOF
