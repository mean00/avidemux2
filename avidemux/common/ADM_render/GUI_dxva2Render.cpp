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
#include "ADM_coreD3DApi.h"
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



#if 0
#define aprintf printf
#else
#define aprintf(...) {}
#endif

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
    \class sdlRender
*/
class dxvaRender: public VideoRenderBase,public ADM_QvideoDrawer
{
  protected:
              bool     useYV12;
  public:
                             dxvaRender( void ) ;
              virtual       ~dxvaRender();
              virtual	  bool init( GUI_WindowInfo *window, uint32_t w, uint32_t h, float zoom);
              virtual	  bool stop(void);
              virtual   bool displayImage(ADMImage *pic);
              virtual   bool changeZoom(float newZoom);
              virtual   bool usingUIRedraw(void) {return false;};
              virtual   bool refresh(void) ;
                        const char *getName() {return "DXVA2";}
                        bool draw(QWidget *widget, QPaintEvent *ev);
                        bool displayImage_argb(ADMImage *pic);
                        bool displayImage_yv12(ADMImage *pic);
                        bool displayImage_surface(ADMImage *pic,admDx2Surface *image);
            //  virtual   ADM_HW_IMAGE getPreferedImage(void ) {return ADM_HW_DXVA;}
                        bool displayImage_surface(admDx2Surface *pic);
               virtual   ADM_HW_IMAGE getPreferedImage(void ) {return ADM_HW_DXVA;}


  protected:
                        admMutex        lock;
                        GUI_WindowInfo  info;
                        IDirect3DSurface9 *mySurface;
                        IDirect3DSurface9 *myYV12Surface;
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
    ADM_info("creating Dxva2/D3D render.\n");
    useYV12=false;
    mySurface=NULL;
    myYV12Surface=NULL;
    videoWidget=NULL;
    d3dHandle=admD3D::getHandle();
}
/**
    \fn dxvaRender
*/
dxvaRender::~dxvaRender()
{
    if(videoWidget)
    {
        videoWidget->useExternalRedraw(false); // reactivate Qt double buffering
        videoWidget->setDrawer(NULL);
        videoWidget=NULL;
    }
    ADM_info("dxva2/D3D clean up\n");
    cleanup();
    ADM_info("dxva2/D3D cleaned up\n");
}

/**
    \fn stop
*/
bool dxvaRender::stop(void)
{
    ADM_info("stopping dxva2/D3D render.\n");
    return true;
}
/**
    \fn changeZoom
*/
bool dxvaRender::changeZoom(float newZoom)
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
bool dxvaRender::init( GUI_WindowInfo *window, uint32_t w, uint32_t h, float zoom)
{
    ADM_info("Initializing dxva2/D3D render\n");
    info=*window;
    baseInit(w,h,zoom);

    windowId=(HWND)window->systemWindowId;

    if(!d3dHandle)
    {
        ADM_warning("No D3DHandle\n");
        return false;
    }

    if (ADM_FAILED(D3DCall(IDirect3D9,GetAdapterDisplayMode,d3dHandle,
                                                D3DADAPTER_DEFAULT,
                                                &displayMode)))
    {
        ADM_warning("Dxv2/D3D Render: Cannot get display mode\n");
        return 0;
    }

    D3DCAPS9 deviceCapabilities;
    ADM_info("D3D Checking device capabilities\n");
    if (ADM_FAILED(D3DCall(IDirect3D9,GetDeviceCaps,d3dHandle,
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
    if (ADM_FAILED(D3DCall(IDirect3D9,CheckDeviceFormatConversion, d3dHandle, // adapter
                                                         D3DADAPTER_DEFAULT, // device type
                                                         D3DDEVTYPE_HAL, // adapter format
                                                         yv12, // render target format
                                                         fmt)))  // depth stencil format
    {
        useYV12=false;
        ADM_info("D3D YV12 not supported\n");
    }
    else
    {
        useYV12=true;
        ADM_info("D3D YV12 is supported\n");
    }



    if(!setup())
    {
      ADM_warning("Dxva/D3D setup failed\n");
      return false;
    }
    videoWidget=(ADM_Qvideo *)info.widget;
    videoWidget->useExternalRedraw(true); // deactivate Qt Double buffering
    videoWidget->setDrawer(this);

    ADM_info("Dxva (D3D) init successful, dxva render. w=%d, h=%d, zoom=%.4f, displayWidth=%d, displayHeight=%d\n",(int)w,(int)h,zoom,(int)displayWidth,(int)displayHeight);
    return true;
}

/**
  \fn setup
  \brief Allocate stuff for a given display with/height. It is called again each time the zoom is changed
*/
bool dxvaRender::setup()
{
  D3DVIEWPORT9 viewPort = {0, 0, displayWidth, displayHeight, 0, 1};


     ADM_info("D3D (re)Setting up \n");
     D3DPRESENT_PARAMETERS presentationParameters;
     memset(&presentationParameters, 0, sizeof(presentationParameters));
     presentationParameters.Windowed               = TRUE;
     presentationParameters.SwapEffect             = D3DSWAPEFFECT_DISCARD; // We could use copy, but discard seems faster according to ms
     presentationParameters.Flags                  = D3DPRESENTFLAG_VIDEO;
     presentationParameters.hDeviceWindow          = windowId;
     presentationParameters.BackBufferWidth        = displayWidth;
     presentationParameters.BackBufferHeight       = displayHeight;
     presentationParameters.MultiSampleType        = D3DMULTISAMPLE_NONE;
     presentationParameters.PresentationInterval   = D3DPRESENT_INTERVAL_ONE;
     presentationParameters.BackBufferFormat       = displayMode.Format;
     presentationParameters.BackBufferCount        = 1;
     presentationParameters.EnableAutoDepthStencil = FALSE;

     if( admD3D::isDirect9Ex())
     {
        IDirect3DDevice9Ex *d3d9deviceex = NULL;
                
        if(ADM_FAILED(D3DCall(IDirect3D9Ex,CreateDeviceEx,  ((IDirect3D9Ex *)d3dHandle),
                                         D3DADAPTER_DEFAULT,
                                         D3DDEVTYPE_HAL,  presentationParameters.hDeviceWindow,
                                         D3DCREATE_SOFTWARE_VERTEXPROCESSING,
                                         &presentationParameters,NULL, &d3d9deviceex)))
        {
          ADM_warning("Failed to create D3D9Ex render device\n");
          d3dDevice=NULL;
        }
        else
        {
           d3dDevice=(IDirect3DDevice9 *)d3d9deviceex;  
           ADM_info("DXVA2Render : device created in D3D9Ex mode\n");
        }
     }

     if(!d3dDevice) // fallback if D3D9Ex fails or not available
     {
       if(ADM_FAILED(D3DCall(IDirect3D9,CreateDevice,  d3dHandle,
                                         D3DADAPTER_DEFAULT,
                                         D3DDEVTYPE_HAL,  presentationParameters.hDeviceWindow,
                                         D3DCREATE_SOFTWARE_VERTEXPROCESSING,
                                         &presentationParameters, &d3dDevice)))
        {
          ADM_warning("dxvaRender::Failed to create D3D render device\n");
          return false;
        }
     }

      //

      D3DFORMAT yv12=(D3DFORMAT)MAKEFOURCC('Y','V','1','2');

      //
       if( ADM_FAILED(D3DCall(IDirect3DDevice9,CreateOffscreenPlainSurface,
                 d3dDevice, displayWidth,displayHeight,
                 displayMode.Format, D3DPOOL_DEFAULT, &mySurface, NULL)))
       {
                  ADM_warning("D3D Cannot create surface\n");
                  return false;
       }
       if( ADM_FAILED(D3DCall(IDirect3DDevice9,CreateOffscreenPlainSurface,
                 d3dDevice, imageWidth,imageHeight,
                 yv12, D3DPOOL_DEFAULT, &myYV12Surface, NULL)))
       {
                  ADM_warning("D3D Cannot create surface\n");
                  return false;
       }
      // put some defaults
      D3DCall(IDirect3DDevice9,SetRenderState,d3dDevice, D3DRS_SRCBLEND, D3DBLEND_ONE);
      D3DCall(IDirect3DDevice9,SetRenderState,d3dDevice, D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
      D3DCall(IDirect3DDevice9,SetRenderState,d3dDevice, D3DRS_ALPHAFUNC, D3DCMP_GREATER);
      D3DCall(IDirect3DDevice9,SetRenderState,d3dDevice, D3DRS_ALPHAREF, (DWORD)0x0);
      D3DCall(IDirect3DDevice9,SetRenderState,d3dDevice, D3DRS_LIGHTING, FALSE);
      D3DCall(IDirect3DDevice9,SetSamplerState,d3dDevice, 0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
      D3DCall(IDirect3DDevice9,SetSamplerState,d3dDevice, 0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
      //


  if(ADM_FAILED(D3DCall(IDirect3DDevice9,SetViewport,d3dDevice, &viewPort)))
  {
      ADM_warning("D3D Cannot set D3D viewport\n");
      return false;
  }

  scaler=new ADMColorScalerFull(ADM_CS_BICUBIC,imageWidth,imageHeight,displayWidth,displayHeight,
          ADM_COLOR_YV12,
          ADM_COLOR_RGB32A
      );
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
  \brief cleanup all display dependant stuff, called again before switching zoom and at exit

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
        D3DCallNoArg(IDirect3DSurface9,Release,mySurface);
        mySurface=NULL;
    }
    if(myYV12Surface)
    {
        D3DCallNoArg(IDirect3DSurface9,Release,myYV12Surface);
        myYV12Surface=NULL;
    }
    if(d3dDevice)
    {
       D3DCallNoArg(IDirect3DDevice9,Release,d3dDevice);
       d3dDevice=NULL;
    }
    return true;
}
 /**
 \fn refresh
 \brief does not work correctly
 */
 bool dxvaRender::refresh(void)
 {
   ADM_info("Refresh**\n");
#if 0
   if( ADM_FAILED(IDirect3DDevice9_Present(d3dDevice, &targetRect, 0, 0, 0)))
   {
        ADM_warning("D3D Present failed\n");
   }
#else
    D3DCallNoArg(IDirect3DDevice9,BeginScene,d3dDevice);
    D3DCallNoArg(IDirect3DDevice9,EndScene,d3dDevice);
    if( ADM_FAILED(D3DCall(IDirect3DDevice9,Present,d3dDevice, &targetRect, 0, 0, 0)))
    {
        ADM_warning("D3D Present failed\n");
    }
#endif
  return true;
 }
/**
  \fn draw
  \brief callback from Qt saying we need to refresh
*/
bool dxvaRender::draw(QWidget *widget, QPaintEvent *ev)
{
  ADM_info("D3D : Draw!\n");
  refresh();
  return true;
}


/**
    \fn d3dBlit
    \brief blit one plane at a time from ADMImahe to surface
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
  \fn ADMImage_To_yv12Surface
*/
static bool  ADMImage_To_yv12Surface(ADMImage *pic, IDirect3DSurface9 *surface)
{
  D3DLOCKED_RECT     lock;;
  if (ADM_FAILED(D3DCall(IDirect3DSurface9,LockRect,surface,&lock, NULL, 0)))
  {
      ADM_warning("D3D Cannot lock surface\n");
      return false;
  }

  // copy
  uint8_t *dst=(uint8_t *)lock.pBits;
  int  dStride=lock.Pitch;

  int width=pic->GetWidth(PLANAR_Y);
  int height=pic->GetHeight(PLANAR_Y);

  d3dBlit(pic, PLANAR_Y,dst,dStride,width,height);

  dst+=height*dStride;
  d3dBlit(pic, PLANAR_V, dst, dStride>>1, width>>1, height>>1);

  dst+=(height/2)*(dStride/2);
  d3dBlit(pic, PLANAR_U, dst, dStride>>1, width>>1, height>>1);

  if (ADM_FAILED(D3DCallNoArg(IDirect3DSurface9,UnlockRect,surface)))
  {
      ADM_warning("D3D Cannot unlock surface\n");
      return false;
  }
  return true;
}
/**
  \fn ADMImage_To_argbSurface
*/
static bool  ADMImage_To_argbSurface(ADMImage *pic, IDirect3DSurface9 *surface,ADMColorScalerFull *scaler)
{
    D3DLOCKED_RECT     lock;

    if (ADM_FAILED(D3DCall(IDirect3DSurface9,LockRect,surface,&lock, NULL, 0)))
    {
        ADM_warning("D3D Cannot lock surface\n");
        return false;
    }
    // RGB
    uint8_t *src[3];
    uint8_t *dst[3];
    pic->GetReadPlanes(src);
    dst[0]=(uint8_t *)lock.pBits;
    dst[1]=dst[2]=NULL;
    int sourcePitch[3],dstPitch[3];
    pic->GetPitches(sourcePitch);
    dstPitch[0]=lock.Pitch;
    dstPitch[1]=dstPitch[2]=0;
    scaler-> convertPlanes(sourcePitch,dstPitch, src, dst);

    if (ADM_FAILED(IDirect3DSurface9_UnlockRect(surface)))
    {
        ADM_warning("D3D Cannot unlock surface\n");
        return false;
    }
    return true;
}

/**
  \fn displayImage_yv12
  \brief copy image to myV12 surface then convert from yv12 to display format in mySurface

*/
bool dxvaRender::displayImage_yv12(ADMImage *pic)
{
    IDirect3DSurface9 *bBuffer;
    // 1 upload to myYV12 surface
    if(!ADMImage_To_yv12Surface(pic,myYV12Surface))
    {
      return false;
    }
   // upload....
    if( ADM_FAILED(IDirect3DDevice9_GetBackBuffer(d3dDevice, 0, 0,
                                              D3DBACKBUFFER_TYPE_MONO,
                                              &bBuffer)))
    {
          ADM_warning("D3D Cannot create backBuffer\n");
          return false;
    }


    // data are in YV12 surface, blit it to mySurface
    // zoom and color conversion happen there

    if (ADM_FAILED(IDirect3DDevice9_StretchRect(d3dDevice,
                    myYV12Surface,
                    NULL,
                    bBuffer,
                    NULL,
                    D3DTEXF_LINEAR)))
                    {
                           ADM_warning("StretchRec yv12 failed\n");
                    }
    IDirect3DDevice9_BeginScene(d3dDevice);
    IDirect3DDevice9_EndScene(d3dDevice);
    if( ADM_FAILED(IDirect3DDevice9_Present(d3dDevice, &targetRect, 0, 0, 0)))
    {
      ADM_warning("D3D Present failed\n");
    }

    return true;
}
/**
  \fn displayImage_argb
  \brief manually do the yv12-> RGB conversion + rescale and the upload to backbuffer
*/
bool dxvaRender::displayImage_argb(ADMImage *pic)
{
  IDirect3DSurface9 *bBuffer;
  // 1 upload to myYV12 surface
  if( ADM_FAILED(IDirect3DDevice9_GetBackBuffer(d3dDevice, 0, 0,
                                            D3DBACKBUFFER_TYPE_MONO,
                                            &bBuffer)))
  {
        ADM_warning("D3D Cannot create backBuffer\n");
        return false;
  }

  if(!ADMImage_To_argbSurface(pic,bBuffer,scaler))
  {
    ADM_warning("Image to argb surface failed\n");
    return false;
  }

  IDirect3DDevice9_BeginScene(d3dDevice);

  IDirect3DDevice9_EndScene(d3dDevice);
  if( ADM_FAILED(IDirect3DDevice9_Present(d3dDevice, &targetRect, 0, 0, 0)))
  {
    ADM_warning("D3D Present failed\n");
  }

  return true;
}
/**
  \fn brief input is already a surface, in yv12 format
*/
bool dxvaRender::displayImage_surface(ADMImage *pic,admDx2Surface *surface)
{
#if 0
  // this does not work, both surfaces are coming from different device
    IDirect3DSurface9 *bBuffer;
    if( ADM_FAILED(D3DCall(IDirect3DDevice9,GetBackBuffer,d3dDevice, 0, 0,
                               D3DBACKBUFFER_TYPE_MONO,
                               &bBuffer)))
    {
        ADM_warning("D3D Cannot create backBuffer\n");
        return false;
    }
    // OK
    ADM_info("surface duplicated\n");

    // can we directly use the surface from dxva ? (can we at all ?)
    if (ADM_FAILED(D3DCall(IDirect3DDevice9,StretchRect,d3dDevice,
                               surface->surface,
                               NULL,
                               bBuffer,
                               NULL,
                               D3DTEXF_LINEAR)))
    {
        ADM_warning("StretchRec yv12 failed\n");
    }else
    {
        IDirect3DDevice9_BeginScene(d3dDevice);
        IDirect3DDevice9_EndScene(d3dDevice);
        if( ADM_FAILED(IDirect3DDevice9_Present(d3dDevice, &targetRect, 0, 0, 0)))
        {
            ADM_warning("D3D Present failed\n");
        }
        return true;
    }
#endif
    // go to indirect route
    if(!pic->hwDownloadFromRef())
    {
        ADM_warning("Failed to download yv12 from dxva\n");
        return false;
    }
    // workaround : use default non bridged path
    if(useYV12)
        return displayImage_yv12(pic);
    return displayImage_argb(pic);
}

/**
  \fn displayImage
  \brief display an image
*/
bool dxvaRender::displayImage(ADMImage *pic)
{
  if(pic->refType==ADM_HW_DXVA)
  {
      aprintf("Source is already a surface (dxva2 input)\n");
      admDx2Surface *surface=(admDx2Surface *)pic->refDescriptor.refHwImage;
      return displayImage_surface(pic,surface);
  }


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
