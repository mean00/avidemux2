#pragma once

//clang-format off
#include <QFrame>
//clang-format on
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0) && QT_VERSION <= QT_VERSION_CHECK(6, 0, 0)
#define USING_QT5
#elif QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#define USING_QT6
#else
#error Please use QT5 or QT6
#endif

enum QT_LINUX_WINDOW_ENGINE
{
    QT_NO_ENGINE,
    QT_X11_ENGINE,
    QT_WAYLAND_ENGINE,
};

QT_LINUX_WINDOW_ENGINE admDetectQtEngine();
