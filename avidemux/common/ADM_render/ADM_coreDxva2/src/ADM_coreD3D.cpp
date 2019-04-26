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
#include "ADM_coreD3DApi.h" // Must be the 1st included
#include "ADM_default.h"
#include "ADM_coreD3D.h"

#ifdef USE_DXVA2

#if 1
#define aprintf printf
#define DUMP_GUID
#else
#define aprintf(...) {}
#endif



#include "ADM_dynamicLoading.h"
#include <map>

#include <d3d9.h>
#include <dxva2api.h>

typedef IDirect3D9*   WINAPI pDirect3DCreate9(UINT);
typedef HRESULT       WINAPI pDirect3DCreate9Ex(UINT,IDirect3D9Ex **);

static bool                  coreD3DWorking=false;

static ADM_LibWrapper        d3dDynaLoader;

static pDirect3DCreate9      *createD3D    = NULL;
static pDirect3DCreate9Ex    *createD3Dex  = NULL;

static IDirect3D9            *d3d9         = NULL;
static IDirect3D9Ex          *d3d9ex       = NULL;
static IDirect3DDevice9      *d3d9device   = NULL;
static IDirect3DDevice9Ex    *d3d9deviceex = NULL;
static admD3D::ADM_vendorID  d3dVendorId  =  admD3D::VENDOR_UNKNOWN;
static int64_t               d3dDriverVersion = 0;

static bool isD3D9Ex=false;


#include <initguid.h>
/**
 */
bool              admD3D::isDirect9Ex()
{
  return isD3D9Ex;
}
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
static bool admD3D_initD3D9Ex(GUI_WindowInfo *x)
{
    UINT adapter = D3DADAPTER_DEFAULT;
    D3DDISPLAYMODEEX      d3ddm;
    D3DPRESENT_PARAMETERS d3dpp = {0};
    HWND windowID=(HWND)x->systemWindowId;
    IDirect3D9Ex *d3d9ex = NULL;
    IDirect3DDevice9Ex *d3d9deviceex = NULL;

    HRESULT hr;
    unsigned int resetToken = 0;
    ADM_info("Probing for D3D9EX support\n");
    // Load d3d9 dll
    if(false==d3dDynaLoader.loadLibrary("d3d9.dll"))
    {
        ADM_warning("Dxva2: Cannot load d3d9.dll\n"); // memleak
        goto failInit9x;
    }
    // Load d3d9 dll
    createD3Dex=(pDirect3DCreate9Ex *)d3dDynaLoader.getSymbol("Direct3DCreate9Ex");
    if(!createD3Dex)
    {
        ADM_warning("Dxva2: Cannot get symbol  Direct3DCreate9Ex\n");
        goto failInit9x;
    }
    hr= createD3Dex(D3D_SDK_VERSION,&d3d9ex);

    if(ADM_FAILED(hr))
    {
        ADM_warning("Dxva2: Cannot gcreate d3d9ex\n");
        goto failInit9x;
    }
    ADM_info("D3D library loaded in D3D9Ex mode, Checking Adapater display mode\n");
    memset(&d3ddm,0,sizeof(d3ddm));
    d3ddm.Size=sizeof(d3ddm);

    hr=D3DCall(IDirect3D9Ex,GetAdapterDisplayModeEx,d3d9ex, adapter,
                                &d3ddm,NULL);
    if(ADM_FAILED(hr))
    {
        ADM_warning("GetAdapterDisplayModeEx failed\n");
        return false;
    }
    ADM_info("Display is %d x %d, fmt=0x%x\n",d3ddm.Width,d3ddm.Height,d3ddm.Format);

    D3DADAPTER_IDENTIFIER9 id;
    hr=D3DCall(IDirect3D9,GetAdapterIdentifier,d3d9ex,  adapter, 0, &id);
    if(ADM_FAILED(hr))
    {
        ADM_warning("GetAdapterIdentifier failed\n");
        d3dVendorId=admD3D::VENDOR_UNKNOWN;
        d3dDriverVersion=0;
    }else
    {
        ADM_info("D3D Device: %s, Vendor: %x, Device: %x, Rev: %x, Driver Version: %" PRId64"\n",
                id.Description, id.VendorId, id.DeviceId, id.Revision, id.DriverVersion.QuadPart);
        d3dDriverVersion=id.DriverVersion.QuadPart;
        switch(id.VendorId)
        {
        case  0x1002: d3dVendorId=admD3D::VENDOR_AMD;break;
        case  0x10DE: d3dVendorId=admD3D::VENDOR_NVIDIA;break;
        case  0x8086: d3dVendorId=admD3D::VENDOR_INTEL;break;
        default:
                      d3dVendorId=admD3D::VENDOR_UNKNOWN;
                      break;
        }
    }



    memset(&d3dpp,0,sizeof(d3dpp));
    d3dpp.Windowed         = TRUE;
    d3dpp.BackBufferWidth  = 640;
    d3dpp.BackBufferHeight = 480;
    d3dpp.BackBufferCount  = 0;
    d3dpp.BackBufferFormat = d3ddm.Format;
    d3dpp.SwapEffect       = D3DSWAPEFFECT_DISCARD;
    d3dpp.Flags            = D3DPRESENTFLAG_VIDEO;

    ADM_info("Creating Device9EX\n");
    hr = D3DCall(IDirect3D9Ex,CreateDeviceEx,d3d9ex,
                                adapter,
                                D3DDEVTYPE_HAL,
                                windowID,
                                D3DCREATE_SOFTWARE_VERTEXPROCESSING | D3DCREATE_MULTITHREADED | D3DCREATE_FPU_PRESERVE,
                                &d3dpp,
                                NULL,
                                &d3d9deviceex);
    if(ADM_FAILED(hr))
    {
        ADM_warning("D3D:Cannot create d3d9Ex device\n");
        goto failInit9x;
    }

    coreD3DWorking=true;
    ADM_info("D3D:d3d9Ex core successfully initialized\n");

    d3d9=(IDirect3D9 *)d3d9ex;
    d3d9device=(IDirect3DDevice9 *)d3d9deviceex;
    isD3D9Ex=true;
    return true;
failInit9x:
      ADM_warning("D3D:D3D init failed\n");
      return false;
}

/**
 * \fn init
 * \brief initialize the low level common part of dxva2
 */
static bool admD3D_initD3D9(GUI_WindowInfo *x)
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
    memset(&d3ddm,0,sizeof(d3ddm));
    memset(&d3dpp,0,sizeof(d3dpp));
    D3DCall(IDirect3D9,GetAdapterDisplayMode,d3d9, adapter, &d3ddm);
    d3dpp.Windowed         = TRUE;
    d3dpp.BackBufferWidth  = 640;
    d3dpp.BackBufferHeight = 480;
    d3dpp.BackBufferCount  = 0;
    d3dpp.BackBufferFormat = d3ddm.Format;
    d3dpp.SwapEffect       = D3DSWAPEFFECT_DISCARD;
    d3dpp.Flags            = D3DPRESENTFLAG_VIDEO;

    hr = D3DCall(IDirect3D9,CreateDevice,d3d9,
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
    isD3D9Ex=false;
    return true;
failInit:
      ADM_warning("D3D:D3D init failed\n");
      return false;
}

bool admD3D::init(GUI_WindowInfo *x)
{
  if(admD3D_initD3D9Ex(x)) return true;
  if(admD3D_initD3D9(x)) return true;
  ADM_info("-- D3D:d3d core failed to initialize\n");
  return false;

}
/**
    \fn isOperationnal
*/
bool admD3D::isOperationnal(void)
{
    if(!coreD3DWorking)
      ADM_warning("This binary has no working D3D support\n");
    return coreD3DWorking;
}
/**

*/
admD3D::ADM_vendorID      admD3D::getVendorID(void)
{
  return d3dVendorId;
}

/**
    \fn getDriverVersion
*/
int64_t admD3D::getDriverVersion(void)
{
    return d3dDriverVersion;
}

/**
 */
bool admD3D::cleanup(void)
{
    ADM_warning("This binary has no DXVA2 support\n");

    if (d3d9device)
    {
        //IDirect3DDevice9_Release(d3d9device);
        D3DCallNoArg(IDirect3DDevice9,Release,d3d9device);
        d3d9device  =NULL;
    }

    if (d3d9)
    {
        //IDirect3D9_Release(d3d9);
        D3DCallNoArg(IDirect3D9,Release,d3d9);
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
