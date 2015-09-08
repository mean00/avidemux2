
/***************************************************************************
    Handle all redraw operation for QT4 
    
    copyright            : (C) 2006/2015 by mean
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


#include <QFrame>


#include <QImage>
#include <QPainter>
#include <QPaintEngine>

/* Probably on unix/X11 ..*/
#ifdef __APPLE__
#include <Carbon/Carbon.h>
#elif !defined(_WIN32)

#if QT_VERSION < QT_VERSION_CHECK(5,0,0) 
        #include <QX11Info> // removed in qt5
#else
extern "C"
{
        extern void *XOpenDisplay(    char *name);
}
#endif


#endif

#include "ADM_assert.h"
#include "T_preview.h"
#include "../ADM_render/GUI_render.h"
#include "../ADM_render/GUI_accelRender.h"
#include "DIA_coreToolkit.h"
    
void UI_QT4VideoWidget(QFrame *host);
static QFrame *hostFrame=NULL;
static uint8_t *lastImage=NULL;
extern QWidget *QuiMainWindows;
int paintEngineType = -1;


void DIA_previewInit(uint32_t width, uint32_t height) {}
uint8_t DIA_previewUpdate(uint8_t *data) {return 1;}
void DIA_previewEnd(void) {}
uint8_t DIA_previewStillAlive(void) {return 1;}
extern void callBackQtWindowDestroyed();

//****************************************************************************************************
/*
  Function to display
  Warning the incoming data are YUV!
  They are translated to RGB32 by the colorconv instance.

*/
static uint8_t *rgbDataBuffer=NULL;
static uint32_t displayW=0,displayH=0;

//****************************************************************************************************
/*
  This is the class that will display the video images.
  It is a base QWidget where the image will be put by painter.

*/
bool ADM_QPreviewCleanup(void)
{
    if(rgbDataBuffer) delete [] rgbDataBuffer;
    rgbDataBuffer=NULL;
    return true;
}
/**
 * 
 * @param z
 */
#ifdef __HAIKU__
ADM_Qvideo::ADM_Qvideo(QWidget *z) : QWidget(z)  {}
#else
ADM_Qvideo::ADM_Qvideo(QWidget *z) : QWidget(z) 
{    
#if QT_VERSION < QT_VERSION_CHECK(5,0,0) 
    setAttribute( Qt::WA_PaintOnScreen, true );
   // Put a transparent background
    //setAutoFillBackground(true);
    QPalette p =  palette();
    QColor color(Qt::black);
    color.setAlpha(0);
    p.setColor( QPalette::Window, color );
    setPalette( p );
   
#else
    setAttribute(Qt::WA_UpdatesDisabled);
    
#endif

} //{setAutoFillBackground(false);}
#endif // Haiku
ADM_Qvideo::~ADM_Qvideo() 
{
    printf("[Qvideo]Destroying QVideo\n");
    callBackQtWindowDestroyed();
}
/**
 * 
 * @param ev
 */
void ADM_Qvideo::paintEvent(QPaintEvent *ev)
{	
     if (paintEngineType == -1)
     {
               QPainter painter(this);

               if (painter.isActive())
                       paintEngineType = painter.paintEngine()->type();

               painter.end();
      }

	if (!displayW || !displayH || !rgbDataBuffer )
		return;

            if (renderExposeEventFromUI())
            {
                    QImage image(rgbDataBuffer,displayW,displayH,QImage::Format_RGB32);
                    QPainter painter(this);
                    if (painter.isActive())
                    {
                        //painter.drawImage(QPoint(0,0),image);
                        const QRect rec=ev->rect();
                        int x=rec.x();
                        int y=rec.y();
                        int w=rec.width();
                        int h=rec.height();
                        painter.drawImage(x,y,image,x,y,w,h);
                        painter.end();
                    }else
                    {
                        printf("Painter inactive!\n");
                    }
            }
}

ADM_Qvideo *videoWindow=NULL;
//****************************************************************************************************
void UI_QT4VideoWidget(QFrame *host)
{
   videoWindow=new ADM_Qvideo(host);
   hostFrame=host;
   videoWindow->resize(hostFrame->size());
   videoWindow->show();
   
}
//*************************
/**
    \brief return pointer to the drawing widget that displays video
*/
void *UI_getDrawWidget(void)
{
  return (void *) videoWindow;
}
/**
    \brief Display to widget in RGB32
*/
void UI_rgbDraw(void *widg,uint32_t w, uint32_t h,uint8_t *ptr)
{
      memcpy(rgbDataBuffer,ptr,w*h*4);
      videoWindow->repaint();
    
}

/**
      \brief Resize the window
*/
void  UI_updateDrawWindowSize(void *win,uint32_t w,uint32_t h)
{
	if(displayW == w && displayH == h && rgbDataBuffer)
		return;

	if(rgbDataBuffer)
		delete[] rgbDataBuffer;

	rgbDataBuffer = new uint8_t[w * h * 4]; // 32 bits / color
	displayW = w;
	displayH = h;

	hostFrame->setMinimumSize(displayW, displayH);
	videoWindow->setMinimumSize(displayW, displayH);	

	hostFrame->resize(displayW, displayH);
	videoWindow->resize(displayW, displayH);

	hostFrame->setMaximumSize(displayW, displayH);
	videoWindow->setMaximumSize(displayW, displayH);	
	UI_purge();

	QuiMainWindows->adjustSize();
	UI_purge();

// Trolltech need to get their act together.  Resizing doesn't work well or the same on all platforms.
#if defined(__APPLE__)
	// Hack required for Mac to resize properly.  adjustSize() just doesn't cut the mustard.
	QuiMainWindows->resize(QuiMainWindows->width() + 1, QuiMainWindows->height() + 1);
#else
	// resizing doesn't work unless called twice on Windows and Linux.
	QuiMainWindows->adjustSize();
#endif

	UI_purge();

	printf("[RDR] Resizing to %u x %u\n", displayW, displayH);
}
/**
      \brief Retrieve info from window, needed for accel layer
*/
void UI_getWindowInfo(void *draw, GUI_WindowInfo *xinfo)
{
    ADM_assert(videoWindow);
    QWidget* widget = videoWindow->parentWidget();
    xinfo->widget = videoWindow;

#if defined(_WIN32)
	xinfo->display=(void *)videoWindow->winId();
#elif defined(__APPLE__)
	#if defined(ADM_CPU_X86_64)
		xinfo->display = (void*)videoWindow->winId();
		xinfo->window = 0;
	#else
		xinfo->display = HIViewGetWindow(HIViewRef(widget->winId()));
		xinfo->window = 0;
	#endif
#else
        #if QT_VERSION < QT_VERSION_CHECK(5,0,0) 
                const QX11Info &info=videoWindow->x11Info();
                xinfo->display=info.display();
                // xinfo->window=videoWindow->winId();
                xinfo->window=videoWindow->winId();
        #else
                xinfo->window=videoWindow->winId();
                xinfo->display=XOpenDisplay(NULL);
        #endif
#endif

	xinfo->x = widget->x();
	xinfo->y = widget->parentWidget()->height() - (widget->y() + displayH);
	xinfo->width = displayW;
	xinfo->height = displayH;
}


//****************************************************************************************************
//EOF 
