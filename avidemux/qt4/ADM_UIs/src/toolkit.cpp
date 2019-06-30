#include <QtCore/QStack>
#include <QApplication>
#include <QDesktopWidget>

#include "ADM_toolkitQt.h"
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
	QRect qrect = QApplication::desktop()->availableGeometry();

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
