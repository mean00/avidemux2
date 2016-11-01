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
#include "../include/ADM_coreDxva2.h"

#ifdef USE_DXVA2
#include "../include/ADM_coreDxva2Internal.h"
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


typedef IDirect3D9* WINAPI pDirect3DCreate9(UINT);
typedef HRESULT     WINAPI pCreateDeviceManager9(UINT *, IDirect3DDeviceManager9 **);

static bool                  coreDxva2Working=false;

static ADM_LibWrapper        d3dDynaLoader;
static ADM_LibWrapper        dxva2DynaLoader;

static pDirect3DCreate9      *createD3D = NULL;
static pCreateDeviceManager9 *createDeviceManager = NULL;

static IDirect3D9                  *d3d9        =NULL;
static IDirect3DDevice9            *d3d9device  =NULL;
static IDirect3DDeviceManager9     *d3d9devmgr  =NULL;
static IDirectXVideoDecoderService *decoder_service=NULL;
static HANDLE                       deviceHandle=INVALID_HANDLE_VALUE;
    
#include <initguid.h>
DEFINE_GUID(IID_IDirectXVideoDecoderService, 0xfc51a551,0xd5e7,0x11d9,0xaf,0x55,0x00,0x05,0x4e,0x43,0xff,0x02);

/**
 * \fn init
 * \brief initialize the low level common part of dxva2
 */
bool admDxva2::init(GUI_WindowInfo *x)
{
    UINT adapter = D3DADAPTER_DEFAULT;   
    D3DDISPLAYMODE        d3ddm;
    D3DPRESENT_PARAMETERS d3dpp = {0};

    HRESULT hr;
    unsigned int resetToken = 0;
    
    // Load d3d9 dll
    if(false==d3dDynaLoader.loadLibrary("d3d9.dll"))
    {
        ADM_warning("Dxva2: Cannot load d3d9.dll\n"); // memleak
        goto failInit;
    }
    // Load d3d9 dll
    if(false==dxva2DynaLoader.loadLibrary("dxva2.dll"))
    {
        ADM_warning("Dxva2: Cannot load dxva2.dll\n");
        goto failInit;
    }
    
    createD3D=(pDirect3DCreate9 *)d3dDynaLoader.getSymbol("Direct3DCreate9");
    if(!createD3D)
    {
        ADM_warning("Dxva2: Cannot get symbol  Direct3DCreate9\n");
        goto failInit;
    }
    createDeviceManager=(pCreateDeviceManager9 *)dxva2DynaLoader.getSymbol("DXVA2CreateDirect3DDeviceManager9");
    if(!createDeviceManager)
    {
        ADM_warning("Dxva2: Cannot get symbol  createDeviceManager\n");
        goto failInit;
    }     
    d3d9 = createD3D(D3D_SDK_VERSION);
    if(!d3d9)
    {
        ADM_warning("Dxva2: Cannot gcreate d3d9\n");
        goto failInit;
    }
    IDirect3D9_GetAdapterDisplayMode(d3d9, adapter, &d3ddm);
    d3dpp.Windowed         = TRUE;
    d3dpp.BackBufferWidth  = 640;
    d3dpp.BackBufferHeight = 480;
    d3dpp.BackBufferCount  = 0;
    d3dpp.BackBufferFormat = d3ddm.Format;
    d3dpp.SwapEffect       = D3DSWAPEFFECT_DISCARD;
    d3dpp.Flags            = D3DPRESENTFLAG_VIDEO;

    hr = IDirect3D9_CreateDevice(d3d9, adapter, D3DDEVTYPE_HAL, GetDesktopWindow(),
                                 D3DCREATE_SOFTWARE_VERTEXPROCESSING | D3DCREATE_MULTITHREADED | D3DCREATE_FPU_PRESERVE,
                                 &d3dpp, &d3d9device);
    if(FAILED(hr))
    {
        ADM_warning("Cannot create d3d9 device\n");
        goto failInit;
    }
    hr = createDeviceManager(&resetToken, &d3d9devmgr);
    if (FAILED(hr)) 
    {
        ADM_warning( "Failed to create Direct3D device manager\n");
        goto failInit;
    }
    hr = IDirect3DDeviceManager9_ResetDevice(d3d9devmgr, d3d9device, resetToken);
    if (FAILED(hr))
    {
        ADM_warning( "Failed to bind Direct3D device to device manager\n");
        goto failInit;
    }
    ADM_info("Dxva2 init step1\n");
    hr = IDirect3DDeviceManager9_OpenDeviceHandle(d3d9devmgr, &deviceHandle);
    if (FAILED(hr)) {
         ADM_warning("Failed to open device handle\n");
        goto failInit;
    }

    hr = IDirect3DDeviceManager9_GetVideoService(d3d9devmgr, deviceHandle, (REFIID )IID_IDirectXVideoDecoderService, (void **)&decoder_service);
    if (FAILED(hr)) {
        ADM_warning("Failed to create IDirectXVideoDecoderService\n");
        goto failInit;
    }

    coreDxva2Working=true;
    ADM_info("Dxva2 core successfully initialized\n");
    return true;
failInit:
    // cleanup
    cleanup();
    return false;
}
  
/**
    \fn isOperationnal
*/
bool admDxva2::isOperationnal(void)
{
    ADM_warning("This binary has no working DXVA2 support\n");
    return coreDxva2Working;
}
bool admDxva2::cleanup(void)
{
    ADM_warning("This binary has no DXVA2 support\n");

    if (decoder_service)
    {
        IDirectXVideoDecoderService_Release(decoder_service);    
        decoder_service=NULL;
    }
    if (d3d9devmgr && deviceHandle != INVALID_HANDLE_VALUE)
    {
        IDirect3DDeviceManager9_CloseDeviceHandle(d3d9devmgr, deviceHandle);
        deviceHandle=INVALID_HANDLE_VALUE;
    }

    if (d3d9devmgr)
    {
        IDirect3DDeviceManager9_Release(d3d9devmgr);
        d3d9devmgr  =NULL;
    }

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
    coreDxva2Working=false;
    
    // small memleak  d3dDynaLoader;
    // small memleak dxva2DynaLoader;    
    return true;
}
/**
 * \fn admLibVa_exitCleanup
 */
bool admDxva2_exitCleanup()
{
    ADM_info("Dxva2 cleanup begin\n");
    admDxva2::cleanup();
    ADM_info("Dxva2 cleanup end\n");
    return true;
}




#endif
// EOF
