#include <stdio.h>
#include <QtCore/QMap>
#include <QtCore/QLocale>
#include <QtCore/QTranslator>
#include <QtGui/QApplication>
#include <QtGui/QWidget>

#ifdef __APPLE__
#include <Carbon/Carbon.h>
#endif
#include <QtGui/QPaintEngine>

#include "ADM_inttype.h"
#include "ADM_files.h"
#include "DIA_uiTypes.h"
extern QWidget *QuiMainWindows;

#define MAX_UNLOADED_MSG_LENGTH 400
static QTranslator qtTranslator;
static QTranslator avidemuxTranslator;
static QMap<QString, char*> *map = NULL;
static bool translatorLoaded = false;

static void loadTranslation(QTranslator *qTranslator, QString translation)
{
	printf("[Locale] Loading language file %s ", translation.toUtf8().constData());	

	if (qTranslator->load(translation))
	{
		QApplication::installTranslator(qTranslator);
		printf("succeeded\n");
	}
	else
		printf("FAILED\n");
}

void initTranslator(void) {}

const char* translate(const char *__domainname, const char *__msgid)
{
	QString msgid = QString(__msgid);

	if (!map)
		map = new QMap<QString, char*>;

	if (!map->contains(msgid))
	{
		QByteArray translatedMessage = QApplication::translate("", __msgid).toUtf8();
		int copyLength = translatedMessage.length() + 1;
		char* buffer = new char[translatorLoaded ? copyLength : MAX_UNLOADED_MSG_LENGTH + 1];

		if (!translatorLoaded && copyLength > MAX_UNLOADED_MSG_LENGTH + 1)
		{
			copyLength = MAX_UNLOADED_MSG_LENGTH;
			buffer[MAX_UNLOADED_MSG_LENGTH] = '\0';
		}

		memcpy(buffer, translatedMessage.constData(), copyLength);

		(*map)[msgid] = buffer;
	}

	return map->value(msgid);
}

#define HIDE_STRING_FROM_QT(domainname, msgid)  QApplication::translate(domainname, msgid) // to hide string from lupdate so a true test can be conducted

void loadTranslator(void)
{
	printf("\n[Locale] Locale: %s\n", QLocale::system().name().toUtf8().constData());

#ifdef __APPLE__
	QString appdir = QCoreApplication::applicationDirPath() + "/../Resources/locale/";
#elif defined(__WIN32)
	QString appdir = QCoreApplication::applicationDirPath() + "/i18n/";
#else
	QString appdir = ADM_getInstallRelativePath("share","avidemux","i18n");
#endif

	loadTranslation(&qtTranslator, appdir + "qt_" + QLocale::system().name());
	loadTranslation(&avidemuxTranslator, appdir + "avidemux_" + QLocale::system().name());
	translatorLoaded = true;

	// Re-translate existing map (to take care of global strings already allocated)
	QMapIterator<QString, char*> mapIterator(*map);

	while (mapIterator.hasNext())
	{
		mapIterator.next();

		QByteArray translatedMessage = QApplication::translate("", mapIterator.key().toAscii().constData()).toUtf8();		
		char *buffer = mapIterator.value();
		int copyLength = translatedMessage.length() + 1;

		if (copyLength > MAX_UNLOADED_MSG_LENGTH + 1)
		{
			copyLength = MAX_UNLOADED_MSG_LENGTH;
			buffer[MAX_UNLOADED_MSG_LENGTH] = '\0';
		}

		memcpy(buffer, translatedMessage.constData(), copyLength);
	}

	printf("[Locale] Test: &Edit -> %s\n\n", HIDE_STRING_FROM_QT("MainWindow", "&Edit").toUtf8().data());
}

void destroyTranslator(void)
{
	if (map)
	{
		QMapIterator<QString, char*> mapIterator(*map);

		while (mapIterator.hasNext())
		{
			mapIterator.next();
			delete(mapIterator.value());
		}

		delete map;
	}
}

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

#if defined(__APPLE__)
	*nativeHandle = (intptr_t)HIViewGetWindow(HIViewRef(QuiMainWindows->winId()));
#else
	*nativeHandle = (intptr_t)QuiMainWindows->winId();
#endif
}

extern int paintEngineType;

const char* getNativeRendererDesc(void)
{
	switch (paintEngineType)
	{
		case QPaintEngine::X11:
			return QT_TR_NOOP("Qt (X11)");
		case QPaintEngine::Windows:
			return QT_TR_NOOP("Qt (MS Windows GDI)");
		case QPaintEngine::CoreGraphics:
			return QT_TR_NOOP("Qt (Mac OS X Quartz 2D)");
		case QPaintEngine::QuickDraw:
			return QT_TR_NOOP("Qt (Mac OS X QuickDraw)");
		case QPaintEngine::OpenGL:
			return QT_TR_NOOP("Qt (OpenGL)");
#if QT_VERSION >= 0x040400
		case QPaintEngine::Direct3D:
			return QT_TR_NOOP("Qt (MS Windows Direct3D)");
#endif
		case QPaintEngine::Raster:
			return QT_TR_NOOP("Qt (Default Raster)");
	}

	return "Qt";
}
