/***************************************************************************
                          gui_render.cpp  -  description

	The final render a frame. The external interface is the same
	whatever the mean (RGB/YUV/Xv)
                             -------------------
    begin                : Thu Jan 3 2002
    copyright            : (C) 2002 by mean
    email                : fixounet@free.fr
 ***************************************************************************/

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
#include "GUI_simpleRender.h"

#ifdef USE_XV
#include "GUI_xvRender.h"
#endif

#ifdef USE_SDL
#include "GUI_sdlRender.h"
#endif

#include "ADM_colorspace.h"
#include "DIA_uiTypes.h"

//_____________________________________
//_____________________________________
static VideoRenderBase *renderer=NULL;
static uint8_t         *accelSurface=NULL;
static bool             spawnRenderer(void);
//_______________________________________

static void         *draw=NULL;
static uint32_t     phyW=0,phyH=0; /* physical window size */
static renderZoom   lastZoom=ZOOM_1_1;
static uint8_t      _lock=0;

static const UI_FUNCTIONS_T *HookFunc=NULL;
//_______________________________________
/**
 *      \fn ADM_renderLibInit
 *      \brief Initialize the renderlib with the needed external functions
 * 
 */
uint8_t ADM_renderLibInit(const UI_FUNCTIONS_T *funcs)
  {
    HookFunc=funcs;
    ADM_assert(funcs->apiVersion==ADM_RENDER_API_VERSION_NUMBER);
    return 1;
  }
//**************************************
//**************************************
//**************************************
#define RENDER_CHECK(x) {ADM_assert(HookFunc);ADM_assert(HookFunc->x);}
void MUI_getWindowInfo(void *draw, GUI_WindowInfo *xinfo)
{
  RENDER_CHECK(UI_getWindowInfo);
  HookFunc->UI_getWindowInfo(draw, xinfo);
}
static void MUI_updateDrawWindowSize(void *win,uint32_t w,uint32_t h)
{
   RENDER_CHECK(UI_updateDrawWindowSize);
   HookFunc->UI_updateDrawWindowSize(win,w,h);
}
void MUI_rgbDraw(void *widg,uint32_t w, uint32_t h,uint8_t *ptr)
{
    RENDER_CHECK(UI_rgbDraw);
    HookFunc->UI_rgbDraw(widg, w,  h,ptr);
  
}
void *MUI_getDrawWidget(void)
{
  RENDER_CHECK(UI_getDrawWidget);
  return HookFunc->UI_getDrawWidget();
}
static   ADM_RENDER_TYPE MUI_getPreferredRender(void)
{
  RENDER_CHECK(UI_getPreferredRender);
  return HookFunc->UI_getPreferredRender();
}
//**************************************
//**************************************
//**************************************

/**
	Render init, initialize internals. Constuctor like function

*/
uint8_t renderInit( void )
{
	draw=MUI_getDrawWidget(  );
	return 1;
}

void renderDestroy(void)
{
    ADM_info("Cleaning up Render\n");
}

/**
    \fn renderLock
    \brief Take the weak lock (i.e. not threadsafe)
*/
uint8_t renderLock(void)
{
  ADM_assert(!_lock);
  _lock=1; 
}
uint8_t renderUnlock(void)
{
  ADM_assert(_lock);
  _lock=0; 
}

/**
	Warn the renderer that the display size is changing

*/
//----------------------------------------
uint8_t renderDisplayResize(uint32_t w, uint32_t h,renderZoom zoom)
{
        bool create=false;
        ADM_info("Render to %"LU"x%"LU" zoom=%d\n",w,h,zoom);
        if(!renderer) create=true;
        else
        {
            if(w!=phyW || h!=phyH) create=true;
        }
        if(create)
        {
            if(renderer) delete renderer;
            renderer=NULL;
            phyW=w;
            phyH=h;
            lastZoom=zoom;
            spawnRenderer();
        }else
        {
            if(lastZoom!=zoom) renderer->changeZoom(zoom);
        }
        lastZoom=zoom;
         int mul;
         switch(zoom)
            {
                    case ZOOM_1_4: mul=1;break;
                    case ZOOM_1_2: mul=2;break;
                    case ZOOM_1_1: mul=4;break;
                    case ZOOM_2:   mul=8;break;
                    case ZOOM_4:   mul=16;break;
                    default : ADM_assert(0);
    
            }
        MUI_updateDrawWindowSize(draw,(w*mul)/4,(h*mul)/4);
        UI_purge();
        return 1;
}
/**
	Update the image and render it
	The width and hiehgt must NOT have changed

*/
//----------------------------------------
uint8_t renderUpdateImage(ADMImage *image)
{
    ADM_assert(!_lock);
    renderer->displayImage(image);
    return 1;
}
/**
	Refresh the image from internal buffer / last image
	Used for example as call back for X11 events

*/
//_______________________________________________
uint8_t renderRefresh(void)
{
      if(_lock) return 1;
      if(renderer)
        renderer->refresh();
      return 1;
}
/**
    \fn renderExpose
*/
uint8_t renderExpose(void)
{
    renderRefresh();
}
/**
    \fn spawnRenderer
    \brief Create renderer according to prefs
*/
bool spawnRenderer(void)
{
        
        int prefRenderer=MUI_getPreferredRender();
        bool r=false;

        GUI_WindowInfo xinfo;
        MUI_getWindowInfo(draw, &xinfo);
        switch(prefRenderer)
        {
#if defined(USE_XV)
       case RENDER_XV:
                renderer=new XvRender();
                r=renderer->init(&xinfo,phyW,phyH,lastZoom);
                if(!r)
                {
                    delete renderer;
                    renderer=NULL;
                    ADM_warning("Xv init failed\n");
                }
                else
                {
                    ADM_info("Xv init ok\n");
                }
                break;
#endif

#if (defined(WIN32) || defined(APPLE) )&&  defined(USE_SDL)
			case RENDER_SDL:
#ifdef __WIN32
			case RENDER_DIRECTX:
#endif
				renderer=new sdlRender();
                r=renderer->init(&xinfo,phyW,phyH,lastZoom);
                if(!r)
                {
                    delete renderer;
                    renderer=NULL;
                    ADM_warning("SDL init failed\n");
                }
                else
                {
                    ADM_info("SDL init ok\n");
                }
                break;
#endif
        }
        if(!renderer)
        {
            ADM_info("Using simple renderer\n");
            renderer=new simpleRender();
            GUI_WindowInfo xinfo;
            MUI_getWindowInfo(draw, &xinfo);

            renderer->init(&xinfo,phyW,phyH,lastZoom);
        }
        return true;
}

/**
    \fn 
*/
uint8_t renderStartPlaying( void )
{
#if 0
char *displ;
unsigned int renderI;
ADM_RENDER_TYPE render;
uint8_t r=0;
	ADM_assert(!accel_mode);
        

        render=MUI_getPreferredRender();
        GUI_WindowInfo xinfo;
        MUI_getWindowInfo(draw, &xinfo);
        switch(render)
        {
        
#if defined(USE_XV)
	       case RENDER_XV:
		accel_mode=new XvAccelRender();
                if(accel_mode->hasHwZoom()) r=accel_mode->init(&xinfo,phyW,phyH);
                else r=accel_mode->init(&xinfo,renderW,renderH);
                if(!r)
		{
			delete accel_mode;
			accel_mode=NULL;
			printf("Xv init failed\n");
		}
		else
		{
			printf("Xv init ok\n");
		}
                break;
#endif

#if defined(USE_SDL)
			case RENDER_SDL:
#ifdef __WIN32
			case RENDER_DIRECTX:
#endif
				accel_mode=new sdlAccelRender();

				if(accel_mode->hasHwZoom()) r=accel_mode->init(&xinfo,phyW,phyH);
                else r=accel_mode->init(&xinfo,renderW,renderH);

                if(!r)
				{
					delete accel_mode;
					accel_mode=NULL;
				}

                break;
#endif

            default:break;
        }

        if(!accel_mode)
        {
                rgbConverter.changeWidthHeight(renderW,renderH);
                printf("No accel used for rendering\n");
        }
        else
        {
           ADM_assert(!accelSurface);
           accelSurface=new uint8_t[ (renderW*renderH*3)>>1];
          
        }
#endif	
	return 1;
}

/**
    \fn renderStopPlaying
*/
uint8_t renderStopPlaying( void )
{      
      return true;
}
//***************************************
/**
    \fn bool calcDisplayFromZoom(renderZoom zoom);
*/
bool VideoRenderBase::calcDisplayFromZoom(renderZoom zoom)
{
        int mul;
         switch(zoom)
            {
                    case ZOOM_1_4: mul=1;break;
                    case ZOOM_1_2: mul=2;break;
                    case ZOOM_1_1: mul=4;break;
                    case ZOOM_2:   mul=8;break;
                    case ZOOM_4:   mul=16;break;
                    default : ADM_assert(0);
    
            }
        displayWidth=(imageWidth*mul)/4;
        displayHeight=(imageHeight*mul)/4;
        return true;
}

/**
    \fn baseInit
*/
bool VideoRenderBase::baseInit(uint32_t w,uint32_t h,renderZoom zoom)
{
        imageWidth=w;
        imageHeight=h;
        currentZoom=zoom;
        calcDisplayFromZoom(zoom);
        return true;
}

//EOF
