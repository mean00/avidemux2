
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
// clang-format off
#include <QApplication>
#include <QFrame>
#include <QImage>
#include <QPaintEngine>
#include <QPainter>
#include <QWindow>
#include <QMainWindow>
#include "ADM_qtx.h"
#if QT_VERSION < QT_VERSION_CHECK(6, 5, 0)
  #if !defined(__APPLE__)
    #include <QWindow>
    #if !defined(_WIN32)
      extern "C" void *XOpenDisplay(char *);
    #endif
  #endif
#else
  #include <QGuiApplication>
  using namespace QNativeInterface;
#endif
#ifdef __APPLE__
#include <Carbon/Carbon.h>
#else
//
#ifndef _WIN32
// Linux
#if defined(HAVE_QPLATFORM_NATIVE)
#define USE_NATIVE_API 
#include <QtGui/qpa/qplatformnativeinterface.h>
#endif
#endif
#endif
//
#include "ADM_assert.h"
#include "DIA_coreToolkit.h"
#include "GUI_render.h"
#include "GUI_accelRender.h"
#include "GUI_ui.h"
#include "T_preview.h"
// clang-format on

void UI_QT4VideoWidget(QFrame *host);
extern QApplication *currentQApplication();
extern QWidget *QuiMainWindows;
static uint32_t displayW = 0, displayH = 0;
static ADM_Qvideo *videoWindow = NULL;

void DIA_previewInit(uint32_t width, uint32_t height)
{
}
uint8_t DIA_previewUpdate(uint8_t *data)
{
    return 1;
}
void DIA_previewEnd(void)
{
}
uint8_t DIA_previewStillAlive(void)
{
    return 1;
}
extern void callBackQtWindowDestroyed();

//****************************************************************************************************
/*
  Function to display
  Warning the incoming data are YUV!
  They are translated to RGB32 by the colorconv instance.
*/
//****************************************************************************************************
/*
  This is the class that will display the video images.
  It is a base QWidget where the image will be put by painter.

*/
bool ADM_QPreviewCleanup(void)
{
    return true;
}
/**
 *
 * @param z
 */
#ifdef __HAIKU__
ADM_Qvideo::ADM_Qvideo(QWidget *z) : QWidget(z)
{
}
#else
ADM_Qvideo::ADM_Qvideo(QFrame *z) : QWidget(z)
{
    useExternalRedraw(true);
    drawer = NULL;
    doOnce = false;
    _width = _height = 0;
    hostFrame = z;
    if (admDetectQtEngine() == QT_WAYLAND_ENGINE)
    {
        setAttribute(Qt::WA_DontCreateNativeAncestors);
        setAttribute(Qt::WA_NativeWindow);
    }
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
    // printf("Paint event\n");
    if (drawer)
    {
        drawer->draw(this, ev);
        return;
    }
    if (!doOnce)
    {
        printf("[QVideo] Using default redraw-------------->\n");
        QPainter painter(this);
        QColor blackColor(Qt::black);
        QImage allBlack(width(), height(), QImage::Format_RGB32);
        allBlack.fill(blackColor);
        painter.drawImage(0, 0, allBlack);
        printf("<--------------[QVideo]/ Using default redraw\n");
        doOnce = true;
    }
    else
    {
        renderExposeEventFromUI();
    }
}
/**
    \fn setADMSize
*/
void ADM_Qvideo::setADMSize(int width, int height)
{
    _width = width;
    _height = height;
#if !defined(__APPLE__) && !defined(_WIN32) && defined(USING_QT6)
    // Work around an issue with Qt 6.2.1 which results in video frame displaying
    // garbage for all subsequently loaded videos after its size was set to zero.
    static uint8_t workaround = 0;
#define RUNTIME_VERSION_CHECKED 1
#define WORKAROUND_NEEDED 2
    if (!(workaround & RUNTIME_VERSION_CHECKED))
    {
        char *runtimeQtVersion = ADM_strdup(qVersion()); // We need the runtime version, not the build version.
        if (runtimeQtVersion && strlen(runtimeQtVersion) > 2)
        {
            char *major = runtimeQtVersion;
            char *minor = strchr(runtimeQtVersion, '.');
            if (minor)
            {
                *minor++ = 0;
                if (!strcmp(major, "6") && strlen(minor) && atoi(minor) > 1)
                    workaround |= WORKAROUND_NEEDED;
            }
        }
        ADM_dealloc(runtimeQtVersion);
        runtimeQtVersion = NULL;
        workaround |= RUNTIME_VERSION_CHECKED;
    }
    if (workaround & WORKAROUND_NEEDED)
    {
        if (width < 1)
            width = 1;
        if (height < 1)
            height = 1;
    }
#endif
    hostFrame->setFixedSize(width, height);
    setFixedSize(width, height);
}

void UI_QT4VideoWidget(QFrame *host)
{
    videoWindow = new ADM_Qvideo(host);
    videoWindow->show();
}
//*************************
/**
    \brief return pointer to the drawing widget that displays video
*/
void *UI_getDrawWidget(void)
{
    return (void *)videoWindow;
}

/**
      \brief Resize the window
*/
void UI_updateDrawWindowSize(void *win, uint32_t w, uint32_t h)
{

    displayW = w;
    displayH = h;

    // Resizing a maximized window results in not refreshed areas where widgets
    // in the maximized state were drawn with Qt5 on Linux, try to avoid this.
    // Instead, resize the window later on restore event if necessary.
    if (!QuiMainWindows->isMaximized())
    {
        UI_setBlockZoomChangesFlag(true);
        UI_resize(w, h);
        UI_setBlockZoomChangesFlag(false);
        UI_setNeedsResizingFlag(false);
    }
    else
    {
        UI_setNeedsResizingFlag(true);
    }
    videoWindow->setADMSize(w, h);
    if (!w || !h)
        QuiMainWindows->update(); // clean up the space previously occupied by the video window on closing
    UI_purge();

    printf("[RDR] Resizing to %u x %u\n", displayW, displayH);
}
/**
 *
 *
 */
static void *myDisplay = NULL;
static uint64_t mySystemWindowId = 0;
static void *myWindowOpaque = NULL;
/*
 * Apple, let them null
 *
 */
#if defined(__APPLE__)
static void systemWindowInfo_once()
{
}
#elif defined(_WIN32)
/*
 * Windows
 */
static void systemWindowInfo_once()
{
    myDisplay = (void *)videoWindow->winId();
    mySystemWindowId = videoWindow->winId();
}

#else // linux
typedef struct _XDisplay Display;
extern "C" Display *XOpenDisplay(const char *display_name);
static void systemWindowInfo_once()
{
    if (myDisplay)
        return;
    // Always refresh surface and system IDs
    switch (admDetectQtEngine())
    {
    case QT_X11_ENGINE: {
        auto *x11App = qApp->nativeInterface<QNativeInterface::QX11Application>();

        if (x11App)
        {
            // To get the Xlib Display pointer (Display*)
            myDisplay = x11App->display();
        }
        else
        {

            myDisplay = XOpenDisplay(NULL);
        }
        ADM_info("found x11 display\n");

        mySystemWindowId = videoWindow->winId();
    }
    break;
    case QT_WAYLAND_ENGINE: {
        mySystemWindowId = 0;
        if (myDisplay)
        {
            QPlatformNativeInterface *native = currentQApplication()->platformNativeInterface();
#ifdef USE_NATIVE_API
            struct wl_surface *wlSurface = static_cast<struct wl_surface *>(
                native->nativeResourceForWindow("surface", videoWindow->windowHandle()));
            ADM_info("[DEBUG] videoWindow=%p, handle=%p, wl_surface=%p\n", videoWindow, videoWindow->windowHandle(),
                     wlSurface);
            myWindowOpaque = wlSurface;
#else
            myWindowOpaque = NULL;
#endif
        }
        else
            myWindowOpaque = NULL;
    }
    break;
    default:
        mySystemWindowId = 0;
        myWindowOpaque = NULL;
        break;
    }
}
#endif
/*
 *
 *
 */
static void systemWindowInfo(GUI_WindowInfo *xinfo)
{
    systemWindowInfo_once(); // Refresh info every time

    QWindow *window = QuiMainWindows->windowHandle();
    if (window)
        xinfo->scalingFactor = (double)window->devicePixelRatio();
    xinfo->systemWindowId = mySystemWindowId;
    xinfo->display = myDisplay;
    xinfo->windowOpaquePointer = myWindowOpaque;
}

/**
      \brief Retrieve info from window, needed for accel layer
*/
void UI_getWindowInfo(void *draw, GUI_WindowInfo *xinfo)
{
    ADM_assert(videoWindow);
    QWidget *widget = videoWindow->parentWidget();
    xinfo->widget = videoWindow;
    // xinfo->windowOpaquePointer = myWindowOpaque;
    xinfo->systemWindowId = 0;
    xinfo->scalingFactor = 1.;
    QPoint localPoint(0, 0);
    QPoint windowPoint;
#if 0
    if (admDetectQtEngine() == QT_WAYLAND_ENGINE)
    {
        videoWindow->winId(); // Force handle creation
        QMainWindow *mw = qobject_cast<QMainWindow *>(videoWindow->window());
        QWidget *ref = mw ? mw->centralWidget() : videoWindow->window();
        if (!ref)
            ref = videoWindow->window();

        QPoint pWindow = videoWindow->mapTo(videoWindow->window(), QPoint(0, 0));
        QWindow *handle = videoWindow->windowHandle();

        // Calculate the workspace origin (below toolbars)
        QPoint pWorkspace = ref->mapTo(videoWindow->window(), QPoint(0, 0));

        // Use the X from the layout but anchor Y to the top of the central area
        windowPoint = QPoint(pWindow.x(), pWorkspace.y());
        ADM_info("[DEBUG] pWindow=(%d, %d), pWorkspace.y=%d, final windowPoint=(%d, %d)\n", pWindow.x(), pWindow.y(),
                 pWorkspace.y(), windowPoint.x(), windowPoint.y());
        if (handle)
        {
            handle->setPosition(windowPoint);
        }
    }
    else
#endif
    windowPoint = videoWindow->mapToGlobal(localPoint);
    xinfo->x = windowPoint.x();
    xinfo->y = windowPoint.y();
    xinfo->width = displayW;
    xinfo->height = displayH;
    systemWindowInfo(xinfo);
}
/**
 * \brief DEPRECATED
 * @return
 */
void UI_rgbDraw(void *widg, uint32_t w, uint32_t h, uint8_t *ptr)
{
    videoWindow->repaint();
}

//****************************************************************************************************
// EOF
