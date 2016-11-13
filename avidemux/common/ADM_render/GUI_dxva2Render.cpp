/**
    \author mean fixounet@free.fr 2016
    \brief d3d/dxva renderer

    Inspired stronly by mplayer vo_direct3d
*/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "config.h"
#include "ADM_default.h"
#include "DIA_coreToolkit.h"
#include "GUI_render.h"
#include "GUI_renderInternal.h"
#include "GUI_accelRender.h"
#include "ADM_threads.h"

#include "ADM_coreD3D.h"
#include <dxva2api.h>
#include <d3d9types.h>
#include "ADM_coreDxva2.h"
#include "../../qt4/ADM_userInterfaces/ADM_gui/T_preview.h"

/**
    \class sdlRender
*/
class dxvaRender: public VideoRenderBase,public ADM_QvideoDrawer
{
  protected:
              bool     useYV12;
  public:
                             dxvaRender( void ) ;
              virtual       ~dxvaRender();
              virtual	  bool init( GUI_WindowInfo *  window, uint32_t w, uint32_t h,renderZoom zoom);
              virtual	  bool stop(void);
              virtual   bool displayImage(ADMImage *pic);
              virtual   bool changeZoom(renderZoom newZoom);
              virtual   bool usingUIRedraw(void) {return false;};
              virtual   bool refresh(void) ;
                        const char *getName() {return "DXVA2";}
                        bool draw(QWidget *widget, QPaintEvent *ev);
                        bool displayImage_argb(ADMImage *pic);
                        bool displayImage_yv12(ADMImage *pic);

  protected:
                        admMutex        lock;
                        GUI_WindowInfo  info;
                        IDirect3DSurface9 *mySurface;
                        IDirect3DSurface9 *myYV12Surface;
                        uint8_t           *videoBuffer;
                        D3DDISPLAYMODE     displayMode;
                        IDirect3DDevice9  *d3dDevice;
                        IDirect3D9        *d3dHandle;
                        D3DLOCKED_RECT     d3dLock;
                        RECT               panScan;
                        RECT               targetRect;
                        ADM_Qvideo        *videoWidget;
                        HWND              windowId;
protected:
                        bool cleanup();
                        bool setup();
};


/**
        \fn RenderSpawnDxva2
*/
VideoRenderBase *RenderSpawnDxva2()
{
    return new dxvaRender();
}

/**
    \fn dxvaRender
*/
dxvaRender::dxvaRender()
{
    ADM_info("creating Dxva2 render.\n");
    useYV12=false;
    mySurface=NULL;
    myYV12Surface=NULL;
    videoBuffer=NULL;
    d3dHandle=admD3D::getHandle();

}
/**
    \fn dxvaRender
*/
dxvaRender::~dxvaRender()
{
    ADM_info("dxva2 clean up\n");
    cleanup();
    ADM_info("dxva2 cleaned up\n");
}

/**
    \fn stop
*/
bool dxvaRender::stop(void)
{
    ADM_info("stopping dxva2 render.\n");
    return true;
}
/**
    \fn changeZoom
*/
bool dxvaRender::changeZoom(renderZoom newZoom)
{
        ADM_info("changing zoom, dxva/d3D render.\n");
        calcDisplayFromZoom(newZoom);
        currentZoom=newZoom;
        cleanup();
        setup();
        return false;
}


/**
    \fn changeZoom
*/
bool dxvaRender::init( GUI_WindowInfo *  window, uint32_t w, uint32_t h,renderZoom zoom)
{
    ADM_info("Initializing D3D render\n");
    info=*window;
    baseInit(w,h,zoom);

    windowId=(HWND)window->systemWindowId;

    if(!d3dHandle)
    {
        ADM_warning("No D3DHandle\n");
        return false;
    }

    if (FAILED(IDirect3D9_GetAdapterDisplayMode(d3dHandle,
                                                D3DADAPTER_DEFAULT,
                                                &displayMode)))
    {
        ADM_warning("Dxv2Render: Cannot get display mode\n");
        return 0;
    }

    D3DCAPS9 deviceCapabilities;
    ADM_info("Checking device capabilities\n");
    if (FAILED(IDirect3D9_GetDeviceCaps(d3dHandle,
                                        D3DADAPTER_DEFAULT,
                                        D3DDEVTYPE_HAL,
                                        &deviceCapabilities)))
    {
      ADM_warning("Cannot get device capabilities");
      return false;
    }
    int texture = deviceCapabilities.TextureCaps;
    ADM_info("Power of 2 : %d\n",  (texture & D3DPTEXTURECAPS_POW2) &&   !(texture & D3DPTEXTURECAPS_NONPOW2CONDITIONAL));
    ADM_info("Square only: %d\n",  (texture & D3DPTEXTURECAPS_SQUAREONLY));



      // Check if we support YV12
    D3DFORMAT fmt=displayMode.Format;
    D3DFORMAT yv12=(D3DFORMAT)MAKEFOURCC('Y','V','1','2');
    if (FAILED(IDirect3D9_CheckDeviceFormatConversion(   d3dHandle, // adapter
                                                         D3DADAPTER_DEFAULT, // device type
                                                         D3DDEVTYPE_HAL, // adapter format
                                                         yv12, // render target format
                                                         fmt)))  // depth stencil format
    {
        useYV12=false;
        ADM_info("YV12 not supported\n");
    }
    else
    {
        useYV12=true;
        ADM_info("YV12 is supported\n");
    }




    videoWidget=(ADM_Qvideo *)info.widget;
    videoWidget->useExternalRedraw(false);
    videoWidget->setDrawer(this);


    if(!setup())
    {
      ADM_warning("Dxva/D3D setup failed\n");
      return false;
    }

    ADM_info("Dxva (D3D) init successful, dxva render. w=%d, h=%d,zoom=%d, displayWidth=%d, displayHeight=%d\n",(int)w,(int)h,(int)zoom,(int)displayWidth,(int)displayHeight);
    return true;
}

/**
*/
bool dxvaRender::setup()
{
  D3DVIEWPORT9 viewPort = {0, 0, displayWidth, displayHeight, 0, 1};


    ADM_info("(re)Setting up \n");
     D3DPRESENT_PARAMETERS presentationParameters;
     memset(&presentationParameters, 0, sizeof(presentationParameters));
     presentationParameters.Windowed               = TRUE;
     presentationParameters.SwapEffect             = D3DSWAPEFFECT_DISCARD;
     presentationParameters.Flags                  = D3DPRESENTFLAG_VIDEO;
     presentationParameters.hDeviceWindow          = windowId;
     presentationParameters.BackBufferWidth        = displayWidth;
     presentationParameters.BackBufferHeight       = displayHeight;
     presentationParameters.MultiSampleType        = D3DMULTISAMPLE_NONE;
     presentationParameters.PresentationInterval   = D3DPRESENT_INTERVAL_ONE;
     presentationParameters.BackBufferFormat       = displayMode.Format;
     presentationParameters.BackBufferCount        = 1;
     presentationParameters.EnableAutoDepthStencil = FALSE;


     if(FAILED(IDirect3D9_CreateDevice(  d3dHandle,
                                         D3DADAPTER_DEFAULT,
                                         D3DDEVTYPE_HAL,  presentationParameters.hDeviceWindow,
                                         D3DCREATE_SOFTWARE_VERTEXPROCESSING,
                                         &presentationParameters, &d3dDevice)))
     {
        ADM_warning("Failed to create D3D device\n");
        return false;
    }


      //

      D3DFORMAT yv12=(D3DFORMAT)MAKEFOURCC('Y','V','1','2');

      //
       if( FAILED(IDirect3DDevice9_CreateOffscreenPlainSurface(
                 d3dDevice, displayWidth,displayHeight,
                 displayMode.Format, D3DPOOL_DEFAULT, &mySurface, NULL)))
       {
                  ADM_warning("Cannot create surface\n");
                  return false;
       }
       if( FAILED(IDirect3DDevice9_CreateOffscreenPlainSurface(
                 d3dDevice, imageWidth,imageHeight,
                 yv12, D3DPOOL_DEFAULT, &myYV12Surface, NULL)))
       {
                  ADM_warning("Cannot create surface\n");
                  return false;
       }
      // put some defaults
      IDirect3DDevice9_SetRenderState(d3dDevice, D3DRS_SRCBLEND, D3DBLEND_ONE);
      IDirect3DDevice9_SetRenderState(d3dDevice, D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
      IDirect3DDevice9_SetRenderState(d3dDevice, D3DRS_ALPHAFUNC, D3DCMP_GREATER);
      IDirect3DDevice9_SetRenderState(d3dDevice, D3DRS_ALPHAREF, (DWORD)0x0);
      IDirect3DDevice9_SetRenderState(d3dDevice, D3DRS_LIGHTING, FALSE);
      IDirect3DDevice9_SetSamplerState(d3dDevice, 0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
      IDirect3DDevice9_SetSamplerState(d3dDevice, 0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
      //


  if(FAILED(IDirect3DDevice9_SetViewport(d3dDevice, &viewPort)))
  {
      ADM_warning("Cannot set D3D viewport\n");
      return false;
  }

 scaler=new ADMColorScalerFull(ADM_CS_BICUBIC,imageWidth,imageHeight,displayWidth,displayHeight,
          ADM_COLOR_YV12,
          ADM_COLOR_RGB32A
      );
  videoBuffer=new uint8_t[displayWidth*displayHeight*4];
  panScan.left  =0;
  panScan.right =imageWidth-1;
  panScan.top   =0;
  panScan.bottom=imageHeight-1;

  targetRect.left  =0;
  targetRect.right =displayWidth-1;
  targetRect.top   =0;
  targetRect.bottom=displayHeight-1;
  ADM_info("Setup done\n");
  return true;
}
/**
  \fn cleanup

*/
bool dxvaRender::cleanup()
{
    ADM_info("D3D cleanup\n");
    if(scaler)
    {
      delete scaler;
      scaler=NULL;
    }
    if(mySurface)
    {
                IDirect3DSurface9_Release(mySurface);
                mySurface=NULL;
    }
    if(myYV12Surface)
    {
      IDirect3DSurface9_Release(myYV12Surface);
      myYV12Surface=NULL;
    }
    if(d3dDevice)
    {
       IDirect3DDevice9_Release(d3dDevice);
       d3dDevice=NULL;
    }
    if(videoBuffer)
    {
      delete [] videoBuffer;
      videoBuffer=NULL;
    }
    return true;


}
 /**
 \fn refresh
 */
 bool dxvaRender::refresh(void)
 {
   RECT rect = {0, 0, displayWidth, displayHeight};
   if(FAILED(IDirect3DDevice9_Present(d3dDevice, &rect, 0, 0, 0)))
   {
     ADM_warning("Refresh failed\n");
   }
   return true;
 }

bool dxvaRender::draw(QWidget *widget, QPaintEvent *ev)
{
  ADM_info("D3D : Draw!\n");
  refresh();
  return true;
}


/**
    \fn displayImage
*/

static bool d3dBlit(ADMImage *pic,ADM_PLANE plane,uint8_t *target,int targetPitch,int w, int h)
{
  uint8_t *y=pic->GetReadPtr(plane);
  int pitch=pic->GetPitch(plane);
  BitBlit(target,targetPitch,
          y,pitch,
          w,h);
  return true;
}
/**

*/
bool dxvaRender::displayImage_yv12(ADMImage *pic)
{
    IDirect3DSurface9 *bBuffer;
    // 1 upload to myYV12 surface
    if (FAILED(IDirect3DSurface9_LockRect(myYV12Surface,&d3dLock, NULL, 0)))
    {
        ADM_warning("Cannot lock surface\n");
        return false;
    }

    // copy
    uint8_t *dst=(uint8_t *)d3dLock.pBits;
    int  dStride=d3dLock.Pitch;

    int width=pic->GetWidth(PLANAR_Y);
    int height=pic->GetHeight(PLANAR_Y);

    d3dBlit(pic, PLANAR_Y,dst,dStride,width,height);

    dst+=height*dStride;
    d3dBlit(pic, PLANAR_U,dst,dStride>>1,width>>1,height>>1);

    dst+=(height/2)*(dStride/2);
    d3dBlit(pic, PLANAR_V,dst,dStride>>1,width>>1,height>>1);

    if (FAILED(IDirect3DSurface9_UnlockRect(myYV12Surface)))
    {
        ADM_warning("Cannot unlock surface\n");
        return false;
    }
   // upload....
    if( FAILED(IDirect3DDevice9_GetBackBuffer(d3dDevice, 0, 0,
                                              D3DBACKBUFFER_TYPE_MONO,
                                              &bBuffer)))
    {
          ADM_warning("Cannot create backBuffer\n");
          return false;
    }


    // data are in YV12 surface, blit it to mySurface
    IDirect3DDevice9_BeginScene(d3dDevice);
    if (FAILED(IDirect3DDevice9_StretchRect(d3dDevice,
                                            myYV12Surface,
                                            NULL,
                                            bBuffer,
                                            NULL,
                                            D3DTEXF_LINEAR)))
                                            {

                                            }
    IDirect3DDevice9_EndScene(d3dDevice);
    if( FAILED(IDirect3DDevice9_Present(d3dDevice, &targetRect, 0, 0, 0)))
    {
      ADM_warning("Present failed\n");
    }

    return true;
}
/**
*/
bool dxvaRender::displayImage_argb(ADMImage *pic)
{
  IDirect3DSurface9 *bBuffer;
  // 1 upload to myYV12 surface
  if( FAILED(IDirect3DDevice9_GetBackBuffer(d3dDevice, 0, 0,
                                            D3DBACKBUFFER_TYPE_MONO,
                                            &bBuffer)))
  {
        ADM_warning("Cannot create backBuffer\n");
        return false;
  }


  if (FAILED(IDirect3DSurface9_LockRect(bBuffer,&d3dLock, NULL, 0)))
  {
      ADM_warning("Cannot lock surface\n");
      return false;
  }
  // RGB
  uint8_t *src[3];
  uint8_t *dst[3];
  pic->GetReadPlanes(src);
  dst[0]=(uint8_t *)d3dLock.pBits;
  dst[1]=dst[2]=NULL;
  int sourcePitch[3],dstPitch[3];
  pic->GetPitches(sourcePitch);
  dstPitch[0]=d3dLock.Pitch;
  dstPitch[1]=dstPitch[2]=0;
  scaler-> convertPlanes(sourcePitch,dstPitch, src, dst);

  if (FAILED(IDirect3DSurface9_UnlockRect(bBuffer)))
  {
      ADM_warning("Cannot unlock surface\n");
      return false;
  }


  IDirect3DDevice9_BeginScene(d3dDevice);

  IDirect3DDevice9_EndScene(d3dDevice);
  if( FAILED(IDirect3DDevice9_Present(d3dDevice, &targetRect, 0, 0, 0)))
  {
    ADM_warning("Present failed\n");
  }

  return true;

}
/**
*/
bool dxvaRender::displayImage(ADMImage *pic)
{
#if 1
  if(useYV12)
#else
 if(0)
#endif
  {
      return displayImage_yv12(pic);
    }
    return displayImage_argb(pic);
}


// EOF
