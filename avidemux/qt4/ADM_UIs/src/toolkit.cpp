#include <QtCore/QStack>
#include <QApplication>
#if QT_VERSION < QT_VERSION_CHECK(5,11,0)
#   include <QDesktopWidget>
#else
#   include <QScreen>
#endif

#include "ADM_toolkitQt.h"
#include "ADM_QSettings.h"
#include "ADM_assert.h"
#include "ADM_default.h"

static QStack<QWidget*> widgetStack;

void qtRegisterDialog(QWidget *dialog)
{
    if (widgetStack.count())
    {
        Qt::WindowFlags flags = dialog->windowFlags();
#if defined(__APPLE__) // && QT_VERSION == QT_VERSION_CHECK(5,10,1)
        //ADM_info("Working around Qt bug introduced in 5.10.1 resulting in non-resizable dialogs with Cocoa\n");

        // Work around a presumable Qt bug which allows application-modal parents of application-modal dialogs to receive focus.
        dialog->setWindowFlag(Qt::Dialog, false);
        dialog->setParent(widgetStack.top(), Qt::Window);
        flags = dialog->windowFlags();
        flags |= Qt::CustomizeWindowHint;
        flags &= ~Qt::WindowMinimizeButtonHint;
        flags &= ~Qt::WindowFullscreenButtonHint;
        dialog->setWindowFlags(flags);
        dialog->setWindowModality(Qt::ApplicationModal);
#else
        bool reparent = false;
        bool isDialog = false;
        if (dialog->parentWidget() != widgetStack.top())
            reparent = true;
        if (flags & Qt::Dialog)
            isDialog = true;
        if (reparent || !isDialog)
        {
            ADM_info("reparenting widget %s\n",dialog->objectName().toUtf8().constData());
            dialog->setParent(widgetStack.top(), Qt::Dialog);
        }
#endif
    }
    widgetStack.push(dialog);
}

void qtUnregisterDialog(QWidget *dialog)
{
	ADM_assert(widgetStack.top() == dialog);
	widgetStack.pop();
}

QWidget* qtLastRegisteredDialog()
{
	if (widgetStack.count())
		return widgetStack.top();
	else
		return NULL;
}

uint8_t UI_getPhysicalScreenSize(void* window, uint32_t *w,uint32_t *h)
{
#if QT_VERSION < QT_VERSION_CHECK(5,11,0)
	QRect qrect = QApplication::desktop()->availableGeometry();
#else
    QRect qrect = QApplication::primaryScreen()->availableGeometry();
#endif
	*w = (uint32_t)qrect.width();
	*h = (uint32_t)qrect.height();
    return 1;
}

// Calculate the zoom ratio required to fit the whole image on the screen.
float UI_calcZoomToFitScreen(QWidget* window, QWidget* canvas, uint32_t imageWidth, uint32_t imageHeight)
{
	int windowWidth, windowHeight;
	int drawingWidth, drawingHeight;
	uint32_t screenWidth, screenHeight;

	windowWidth = window->frameSize().width();
	windowHeight = window->frameSize().height();

	drawingWidth = canvas->frameSize().width();
	drawingHeight = canvas->frameSize().height();

	UI_getPhysicalScreenSize(window, &screenWidth, &screenHeight);

	// Take drawing area out of the equation, how much extra do we need for additional controls?
	windowWidth -= drawingWidth;
	windowHeight -= drawingHeight;

	// This is the true amount of screen real estate we can work with
	screenWidth -= windowWidth;
	screenHeight -= windowHeight;

	// Calculate zoom ratio
	if (imageWidth > screenWidth || imageHeight > screenHeight)
	{
            //return 1;
        }
        float widthRatio = (float)screenWidth / (float)imageWidth;
        float heightRatio = (float)screenHeight / (float)imageHeight;

        float r= (widthRatio < heightRatio ? widthRatio : heightRatio);
        return r;
		
}

/**
 * \fn qtSettingsCreate
 * \brief Returns a pointer to QSettings object, the caller is responsible for deleting it after use.
 */
QSettings *qtSettingsCreate(void)
{
    QString path = ADM_getBaseDir();
    path += "QtSettings.ini";
    QSettings *settings = new QSettings(path, QSettings::IniFormat);
    return settings;
}
