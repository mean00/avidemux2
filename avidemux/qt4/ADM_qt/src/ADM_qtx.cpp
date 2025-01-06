#include "ADM_qtx.h"
#include "ADM_default.h"
#include <QApplication>
#include <QWindow>
static bool engineDetected = false;
static QT_LINUX_WINDOW_ENGINE qtEngine = QT_NO_ENGINE;
extern QApplication *currentQApplication();
/**
 *
 *
 */
#ifdef USING_QT6
QT_LINUX_WINDOW_ENGINE admDetectQtEngine()
{
    if (engineDetected)
        return qtEngine;
    ADM_info("Running on platform %s\n", currentQApplication()->platformName().toLatin1().data());
    auto x11 = currentQApplication()->nativeInterface<QNativeInterface::QX11Application>();
    if (x11)
    {
        ADM_info("Running on X11\n");
        qtEngine = QT_X11_ENGINE;
    }
    else
    {
        auto wayland = currentQApplication()->nativeInterface<QNativeInterface::QWaylandApplication>();
        if (wayland)
        {
            ADM_info("Running on Wayland\n");
            qtEngine = QT_WAYLAND_ENGINE;
        }
    }
    return qtEngine;
}
#else
// QT4 or 5 are only supporting X11
QT_LINUX_WINDOW_ENGINE admDetectQtEngine()
{
    return QT_X11_ENGINE;
}

#endif
