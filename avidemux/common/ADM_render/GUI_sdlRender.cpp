/***************************************************************************
    copyright            : (C) 2006 by mean
    email                : fixounet@free.fr
***************************************************************************/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************///
//  The pitch stride has to be changed as there could be some padding depending on the OS
//
//

#include "config.h"

#if defined(USE_SDL)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern "C" {
// get rid of warnings due to different definitions
#undef HAVE_INTTYPES_H
#undef HAVE_MALLOC_H
#undef HAVE_STDINT_H
#undef HAVE_SYS_TYPES_H
#include "SDL.h"
#include "SDL_syswm.h"
}

#include "ADM_colorspace.h"
#include "GUI_render.h"
#include "GUI_accelRender.h"
#include "GUI_sdlRender.h"
#include "ADM_assert.h"

#ifdef __APPLE__
extern "C"
{
	void initSdlCocoaView(void* parent, int x, int y, int width, int height, bool carbonParent);
	void destroyCocoaView(void);
}
#endif

//******************************************
static uint8_t sdl_running=0;
static SDL_Overlay *sdl_overlay=NULL;
static SDL_Surface *sdl_display=NULL;
static SDL_Rect disp;
#ifdef __WIN32
HWND sdlWin32;
#endif
static ColBase *color=NULL;

sdlAccelRender::sdlAccelRender( void)
{
        useYV12=1;
        decoded=NULL;
}
uint8_t sdlAccelRender::end( void)
{
        if(sdl_overlay)
        {
                SDL_FreeYUVOverlay(sdl_overlay);
        }
        if(sdl_display)
        {
        		SDL_UnlockSurface(sdl_display);
                SDL_FreeSurface(sdl_display);
        }
        if(sdl_running)
        {
                SDL_QuitSubSystem(SDL_INIT_VIDEO);

#ifdef __APPLE__
				destroyCocoaView();
#endif
        }
        if(decoded)
        {
		        delete [] decoded;
		        decoded=NULL;
        }
        sdl_running=0;
        sdl_overlay=NULL;
        sdl_display=NULL;
        printf("[SDL] Video subsystem closed and destroyed\n");
}
uint8_t sdlAccelRender::init( GUI_WindowInfo * window, uint32_t w, uint32_t h)
{
	printf("[SDL] Initialising video subsystem\n");

#ifdef __APPLE__
	if (window->width > w && window->height > h)
	{
		printf("[SDL] Disabling acceleration.  Zoom increase not supported on Mac\n");
		return 0;
	}
#endif

	int bpp;
	int flags;

    // Ask for the position of the drawing window at start
    disp.w=w;
    disp.h=h;
    disp.x=0;
    disp.y=0;

    if(!useYV12)
    {
		color=new ColBase(720,480);
		decoded=new uint8_t[w*h*2];
    }

    // Hack to get SDL to use GTK window, ugly but works
#if !defined(__WIN32) && !defined(__APPLE__)
	char SDL_windowhack[32];
    int winId=(int)window->window;

    sprintf(SDL_windowhack,"SDL_WINDOWID=%"LD,winId);
    putenv(SDL_windowhack);
#endif

    if (SDL_InitSubSystem(SDL_INIT_VIDEO) < 0)
    {
		printf("[SDL] FAILED initialising video subsystem\n");
		printf("[SDL] ERROR: %s\n", SDL_GetError());

        return 0;
    }

    // Do it twice as the 1st time does not work
    // Hack to get SDL to use GTK window, ugly but works
#if !defined(__WIN32) && !defined(__APPLE__)
    putenv(SDL_windowhack);
    SDL_QuitSubSystem(SDL_INIT_VIDEO);
    if (SDL_InitSubSystem(SDL_INIT_VIDEO) < 0)
    {
                printf("[SDL] FAILED initialising video subsystem\n");
                printf("[SDL] ERROR: %s\n", SDL_GetError());
                return 0;
    }
#endif

    sdl_running=1;
    flags = SDL_ANYFORMAT | SDL_HWPALETTE | SDL_HWSURFACE | SDL_NOFRAME;
    bpp= SDL_VideoModeOK( w, h,  32, flags );

#ifdef __WIN32
	// SDL window is created and displayed before we get a chance to set the parent.
	// Therefore, align the SDL overlay with the client area before it is displayed.
	POINT screenPoint = {};
	char origin[43];

	ClientToScreen((HWND)window->display, &screenPoint);
	snprintf(origin, 43, "SDL_VIDEO_WINDOW_POS=%i,%i", screenPoint.x, screenPoint.y);
	putenv(origin);
#endif

#ifdef __APPLE__
	void* parent;

	if (window->display)
		// Carbon parent (Qt4)
		parent = window->display;
	else
		// Cocoa parent (GTK)
		parent = (void*)window->window;

	if (parent)
		// Create Cocoa view and attach to Carbon window using custom Objective-C function.
		// It's a retarded way of doing things but that's what Apple has imposed...
		initSdlCocoaView(parent, window->x, window->y, window->width, window->height, (window->display != NULL));
#endif

	// SDL will resize our window to width and height passed to SetVideoMode.
	// This is fine until we use zoomed views so pass window dimensions instead.
	sdl_display= SDL_SetVideoMode(window->width, window->height, bpp, flags);

    if (!sdl_display)
    {
        end();
        printf("[SDL] Cannot create surface\n");
		printf("[SDL] ERROR: %s\n", SDL_GetError());
        return 0;
    }

    SDL_LockSurface(sdl_display);

#ifdef __WIN32
	struct SDL_SysWMinfo wmInfo;
	SDL_VERSION(&wmInfo.version);

	if (-1 != SDL_GetWMInfo(&wmInfo))
	{
		sdlWin32 = wmInfo.window;

		// Make SDL window a child to prevent it from gaining focus
		int windowFlags = GetWindowLongPtr(sdlWin32, GWL_STYLE);

		SetWindowLongPtr(sdlWin32, GWL_STYLE, (windowFlags & ~WS_POPUP) | WS_CHILD);

		// Set the SDL window's parent to the main window and reposition
		SetParent(sdlWin32, (HWND)window->display);
		SetWindowPos(sdlWin32, NULL, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

		// The SDL window stole focus before it was made a child, so set focus back to the main window
		SetFocus((HWND)window->display);
	}
	else
	{
		printf("[SDL] Reparenting failed\n");
	}
#endif

        int cspace;

        if(useYV12) cspace=SDL_YV12_OVERLAY;
            else    cspace=SDL_YUY2_OVERLAY;
        //_______________________________________________________
        sdl_overlay=SDL_CreateYUVOverlay((w),(h), cspace, sdl_display);

		// DirectX may fail but overlay still created.
		// Not a showstopper but log failure.
		if (strlen(SDL_GetError()))
		{
			printf("[SDL] ERROR: %s\n", SDL_GetError());
		}

        if(!sdl_overlay)
        {
			end();
			printf("[SDL] Cannot create SDL overlay\n");
			printf("[SDL] ERROR: %s\n", SDL_GetError());

			return 0;
        }

        printf("[SDL] Overlay created; type: %d, planes: %d, pitch: %d\n", sdl_overlay->hw_overlay, sdl_overlay->planes, sdl_overlay->pitches[0]);

        if(!sdl_overlay->hw_overlay)
            printf("[SDL] Hardware acceleration disabled\n");

        if(!useYV12) color->reset(w,h);

		printf("[SDL] Video subsystem initalised successfully\n");

        return 1;
}

static void interleave(uint8_t *dst,uint8_t *src,int width, int stride, int lines)
{
    for(int y=0;y<lines;y++)
    {
        memcpy(dst,src,width);
        src+=width;
        dst+=stride;
    }
}
uint8_t sdlAccelRender::display(uint8_t *ptr, uint32_t w, uint32_t h,renderZoom zoom)
{
#ifdef __WIN32
	// DirectX playback doesn't refresh correctly if the parent window is moved.
	// Detect when the parent window has moved and force a coordinate update.
	if (strcmp(getenv("SDL_VIDEODRIVER"), "directx") == 0)
	{
		static RECT lastPos;

		RECT currentPos;
		GetWindowRect(sdlWin32, &currentPos);

		if (currentPos.left != lastPos.left || currentPos.top != lastPos.top)
		{
			// By default SetWindowPos doesn't work if the new coordinates are the same as the
			// current so use SWP_FRAMECHANGED to force an update.
			SetWindowPos(sdlWin32, NULL, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
			lastPos = currentPos;
		}
	}
#endif

int pitch;
int page=w*h;
        ADM_assert(sdl_overlay);
        SDL_LockYUVOverlay(sdl_overlay);
        pitch=sdl_overlay->pitches[0];
//	printf("SDL: new pitch :%d\n",pitch);
        if(useYV12)
        {
	        if(pitch==w)
	            memcpy(sdl_overlay->pixels[0],ptr,w*h);
	        else
	            interleave(sdl_overlay->pixels[0],ptr,w,pitch,h);

	        pitch=sdl_overlay->pitches[1];
	        if(pitch==(w>>1))
	            memcpy(sdl_overlay->pixels[1],ptr+page,(w*h)>>2);
	        else
	            interleave(sdl_overlay->pixels[1],ptr+page,w>>1,pitch,h>>1);

	        pitch=sdl_overlay->pitches[2];
	        if(pitch==(w>>1))
	            memcpy(sdl_overlay->pixels[2],ptr+(page*5)/4,(w*h)>>2);
	        else
	            interleave(sdl_overlay->pixels[2],ptr+(page*5)/4,w>>1,pitch,h>>1);
        }else
        {
	        color->reset(w,h);
	        if(pitch==2*w)
	        {
	            color->scale(ptr,sdl_overlay->pixels[0]);
	        }
	        else
	        {
	            color->scale(ptr,decoded);
	            interleave(sdl_overlay->pixels[0],decoded,2*w,pitch,h);
	        }
        }
        uint32_t factor=4;
               switch(zoom)
               {
                   case ZOOM_1_4: factor=1;break;
                   case ZOOM_1_2: factor=2;break;
                   case ZOOM_1_1: factor=4;break;
                   case ZOOM_2:   factor=8;break;
                   case ZOOM_4:   factor=16;break;
                   default : ADM_assert(0);

               }
        disp.w=(w*factor)/4;
        disp.h=(h*factor)/4;
        disp.x=0;
        disp.y=0;

        SDL_UnlockYUVOverlay(sdl_overlay);
        SDL_DisplayYUVOverlay(sdl_overlay,&disp);

        return 1;
}

void initSdl(int videoDevice)
{
	printf("\n");
	quitSdl();

    int sdl_version = (SDL_Linked_Version()->major*1000)+(SDL_Linked_Version()->minor*100) + (SDL_Linked_Version()->patch);

    printf("[SDL] Version: %u.%u.%u\n",SDL_Linked_Version()->major, SDL_Linked_Version()->minor, SDL_Linked_Version()->patch);

#ifdef __WIN32
	if(videoDevice == RENDER_DIRECTX)
	{
		printf("[SDL] Setting video driver to Microsoft DirectX\n");
		putenv("SDL_VIDEODRIVER=directx");
	}
	else
	{
		printf("[SDL] Setting video driver to Microsoft Windows GDI\n");
		putenv("SDL_VIDEODRIVER=windib");
	}
#endif

	uint32_t sdlInitFlags;

	if (sdl_version > 1209)
		sdlInitFlags = SDL_INIT_EVERYTHING;
	else
		sdlInitFlags = 0;

	printf("[SDL] Initialisation ");

	if (SDL_Init(sdlInitFlags) == 0)
	{
		printf("succeeded\n");

		char driverName[100];

		if (SDL_VideoDriverName(driverName, 100) != NULL)
		{
			printf("[SDL] Video Driver: %s\n", driverName);
		}
	}
	else
	{
		printf("FAILED\n");
		printf("[SDL] ERROR: %s\n", SDL_GetError());
	}

	printf("\n");
}

void quitSdl(void)
{
	if (SDL_WasInit(SDL_INIT_EVERYTHING))
	{
		printf("[SDL] Quitting...\n");
		SDL_Quit();
	}
}
#endif
