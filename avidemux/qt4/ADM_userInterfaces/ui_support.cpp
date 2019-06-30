#include <stdio.h>
#include <QtCore/QUrl>
#include <QDesktopServices>
#include <QWidget>
#include <QtCore/QString>
#ifdef _WIN32
#include <QFile>
#endif
#ifdef __APPLE__
#include <Carbon/Carbon.h>
#endif
#include <QtGui/QPaintEngine>
#include "ADM_default.h"
#include "ADM_inttype.h"
#include "ADM_files.h"
#include "DIA_uiTypes.h"
#include "translator.cpp"

extern QWidget *QuiMainWindows;

void getUIDescription(char* desc)
{
	sprintf(desc, "Qt (%s)", qVersion());
}
ADM_UI_TYPE UI_GetCurrentUI(void)
{
  return ADM_UI_QT4;
}

void getMainWindowHandles(intptr_t *handle, intptr_t *nativeHandle)
{
	*handle = (intptr_t)QuiMainWindows;

#if defined(__APPLE__) && !defined(ADM_CPU_X86_64)
	*nativeHandle = (intptr_t)HIViewGetWindow(HIViewRef(QuiMainWindows->winId()));
#else
	*nativeHandle = (intptr_t)QuiMainWindows->winId();
#endif
}

extern int paintEngineType;

const char* getNativeRendererDesc(int engine)
{
#if 0
	switch (engine)
	{
		case QPaintEngine::X11:
			return QT_TRANSLATE_NOOP("uisupport","X11");
		case QPaintEngine::Windows:
			return QT_TRANSLATE_NOOP("uisupport","MS Windows GDI");
		case QPaintEngine::CoreGraphics:
			return QT_TRANSLATE_NOOP("uisupport","Mac OS X Quartz 2D");
		case QPaintEngine::QuickDraw:
			return QT_TRANSLATE_NOOP("uisupport","Mac OS X QuickDraw");
		case QPaintEngine::OpenGL:
			return QT_TRANSLATE_NOOP("uisupport","OpenGL");
#if QT_VERSION >= 0x040400
		case QPaintEngine::Direct3D:
			return QT_TRANSLATE_NOOP("uisupport","MS Windows Direct3D");
#endif
		case QPaintEngine::Raster:
			return QT_TRANSLATE_NOOP("uisupport","Default Raster");
	}
#endif
	return "Qt";
}

#ifdef _MSC_VER
 #define FLUSH_LOG_BUFFER fflush(stdout);
#else
 #define FLUSH_LOG_BUFFER
#endif

void GUI_OpenApplicationLog()
{
#ifdef _WIN32
    FLUSH_LOG_BUFFER
    QDesktopServices::openUrl(QUrl::fromLocalFile(QString(ADM_getLogDir()) + "admlog.txt"));
#else
    QDesktopServices::openUrl(QUrl::fromLocalFile(QString(ADM_getBaseDir()) + "admlog.txt"));
#endif
}

void GUI_OpenApplicationDataFolder()
{
    QString baseDir = QString(ADM_getBaseDir());
#ifdef _WIN32
    QString logDir = QString(ADM_getLogDir());
    if(logDir != baseDir)
    {
        QString src = logDir + QString("admlog.txt");
        QString old = baseDir + QString("admlog_old.txt");
        bool r = true;
        if(QFile::exists(old))
            r = QFile::remove(old);
        QString dest = baseDir + QString("admlog.txt");
        if(r && QFile::exists(dest))
            QFile::copy(dest,old);
        if(QFile::exists(dest) && !QFile::remove(dest))
            ADM_warning("Could not delete %s to copy %s there\n",dest.toUtf8().constData(),src.toUtf8().constData());
        else
        {
            FLUSH_LOG_BUFFER
            if(!QFile::copy(src,dest))
                ADM_warning("Copying %s to %s failed\n",src.toUtf8().constData(),dest.toUtf8().constData());
        }
    }
#endif
    QDesktopServices::openUrl(QUrl::fromLocalFile(baseDir));
}
