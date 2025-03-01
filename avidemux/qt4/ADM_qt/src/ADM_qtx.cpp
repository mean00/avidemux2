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
#if defined(__APPLE__) || defined(_WIN32)
QT_LINUX_WINDOW_ENGINE admDetectQtEngine()
{
    return QT_NO_ENGINE;
}

#else
#ifdef USING_QT6
QT_LINUX_WINDOW_ENGINE admDetectQtEngine()
{
    if (engineDetected)
        return qtEngine;
    QString pname = currentQApplication()->platformName();
    ADM_info("Running on platform %s\n", pname.toLatin1().constData());
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
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
#else
    if (!strncmp(pname.toLatin1().constData(), "xcb", 3))
        qtEngine = QT_X11_ENGINE;
    else if (!strncmp(pname.toLatin1().constData(), "wayland", 7))
        qtEngine = QT_WAYLAND_ENGINE;
#endif
    return qtEngine;
}
#else
// QT4 or 5 are only supporting X11
QT_LINUX_WINDOW_ENGINE admDetectQtEngine()
{
    return QT_X11_ENGINE;
}

#endif
#endif
