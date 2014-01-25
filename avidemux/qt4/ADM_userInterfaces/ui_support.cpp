#include <stdio.h>
#include <QtCore/QMap>
#include <QtCore/QLocale>
#include <QtCore/QTranslator>
#include <QtCore/QUrl>
#include <QtGui/QApplication>
#include <QtGui/QDesktopServices>
#include <QtGui/QWidget>

#ifdef __APPLE__
#include <Carbon/Carbon.h>
#endif
#include <QtGui/QPaintEngine>
#include "ADM_default.h"
#include "ADM_inttype.h"
#include "ADM_files.h"
#include "DIA_uiTypes.h"
#include "ADM_coreTranslator.h"
#include "prefs.h"
extern QWidget *QuiMainWindows;

#define MAX_UNLOADED_MSG_LENGTH 400
static QTranslator qtTranslator;
static QTranslator avidemuxTranslator;
static QMap<QString, char*> *map = NULL;
static bool translatorLoaded = false;

static int loadTranslation(QTranslator *qTranslator, QString translation)
{
	ADM_info("[Locale] Loading language file %s ", translation.toUtf8().constData());

	if (qTranslator->load(translation))
	{
		QApplication::installTranslator(qTranslator);
		ADM_info("succeeded\n");
        return 1;
	}
    ADM_warning("FAILED\n");
    return 0;
}

const char* qt4Translate(const char *__domainname, const char *__msgid)
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
/**
 * \fn initTranslator
 */
void initTranslator(void) 
{
    ADM_InitTranslator(qt4Translate);
}

#define HIDE_STRING_FROM_QT(domainname, msgid)  QApplication::translate(domainname, msgid) // to hide string from lupdate so a true test can be conducted

void loadTranslator(void)
{
	
        char *lang=NULL;
        bool autoSelect=true;
        if(prefs->get(DEFAULT_LANGUAGE,&lang))
        {
            if(lang && strlen(lang)>0 && strcmp(lang,"auto"))
                autoSelect=false;
        }
        if(autoSelect)
        {
            ADM_info("Using system language\n");
            lang=ADM_strdup(QLocale::system().name().toUtf8().constData());
        }else
        {
            ADM_info("Language forced \n");
        }
        ADM_info("Initializing language %s\n",lang);
#ifdef __APPLE__
	QString appdir = QCoreApplication::applicationDirPath() + "/../share/avidemux6/i18n/";
                
#elif defined(_WIN32)
	QString appdir = QCoreApplication::applicationDirPath() + "/i18n/";
#else
	QString appdir = ADM_getInstallRelativePath("share","avidemux6","i18n");
#endif
        QString languageFile=QString(lang);
    int nbLoaded=0;
	nbLoaded+=loadTranslation(&qtTranslator, appdir + "qt_" + languageFile);
	nbLoaded+=loadTranslation(&avidemuxTranslator, appdir + "avidemux_" + languageFile);
	translatorLoaded = true;
    if(!nbLoaded) // Nothing to translate..
        return;
    ADM_info("Updating translations...\n");
	// Re-translate existing map (to take care of global strings already allocated)
        if(!map)
            map = new QMap<QString, char*>;
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

	ADM_info("[Locale] Test: &Edit -> %s\n\n", HIDE_STRING_FROM_QT("MainWindow", "&Edit").toUtf8().data());
        
}

void destroyTranslator(void)
{
	if (map)
	{
		QMapIterator<QString, char*> mapIterator(*map);

		while (mapIterator.hasNext())
		{
			mapIterator.next();
			delete [] mapIterator.value();
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

#if defined(__APPLE__) && !defined(ADM_CPU_X86_64)
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

	return "Qt";
}

void GUI_OpenApplicationLog()
{
	QDesktopServices::openUrl(QUrl::fromLocalFile(QString(ADM_getBaseDir()) + "admlog.txt"));
}

void GUI_OpenApplicationDataFolder()
{
	QDesktopServices::openUrl(QUrl::fromLocalFile(QString(ADM_getBaseDir())));
}
