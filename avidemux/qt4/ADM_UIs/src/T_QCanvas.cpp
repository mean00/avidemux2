/***************************************************************************
    copyright            : (C) 2006 by mean
    email                : fixounet@free.fr
***************************************************************************/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************///

#include <QPainter>
#include <QGraphicsView>
#include <QSlider>

#include "config.h"

#ifdef USE_OPENGL
    #include "ADM_openGl.h"
#endif

#include "ADM_default.h"
#include "DIA_flyDialogQt4.h"
#include "prefs.h"

void ADM_QCanvas::changeSize(uint32_t w,uint32_t h)
{
	_w=w;
	_h=h;
#if QT_VERSION >= QT_VERSION_CHECK(5,5,0)
	_w=(_w*devicePixelRatioF() + 0.5);
	_h=(_h*devicePixelRatioF() + 0.5);
#endif
	_l=ADM_IMAGE_ALIGN(_w*4);
	dataBuffer=NULL;
#ifdef USE_OPENGL
	QtGlAccelWidget *gl = (QtGlAccelWidget *)accel;
	if(gl)
		gl->setDisplaySize(w,h);
#endif
	resize(w,h);
}

void ADM_QCanvas::getDisplaySize(uint32_t *w,uint32_t *h)
{
	*w=_w;
	*h=_h;
}

ADM_QCanvas::ADM_QCanvas(QWidget *z,uint32_t w,uint32_t h) : QWidget(z) 
{
	accel = NULL;
	changeSize(w,h);
}

ADM_QCanvas::~ADM_QCanvas()
{
	uninitAccel();
}

bool ADM_QCanvas::initAccel(void)
{
#ifdef USE_OPENGL
    char *noaccel = getenv("ADM_QCANVAS_NOACCEL");
    if(noaccel && !strcmp("1",noaccel))
        return false;
    if(!ADM_glHasActiveTexture())
        return false;
    bool r = false;
    if(!prefs->get(FEATURES_ENABLE_OPENGL,&r) || !r)
        return false;
    QtGlAccelWidget *gl = new QtGlAccelWidget(this, width(), height());
    gl->setDisplaySize(width(), height());
    gl->show();
    r = QOpenGLShaderProgram::hasOpenGLShaderPrograms(gl->context());
    printf("[ADM_QCanvas::initAccel] Init %s\n", r? "succeeded" : "failed: OpenGL shader program not supported");
    gl->doneCurrent();
    accel = (void *)gl;
    return r;
#else
    return false;
#endif
}

void ADM_QCanvas::uninitAccel(void)
{
#ifdef USE_OPENGL
    QtGlAccelWidget *gl = (QtGlAccelWidget *)accel;
    if(!gl) return;
    gl->setParent(NULL);
    delete gl;
    accel = NULL;
#endif
}

bool ADM_QCanvas::displayImage(ADMImage *pic)
{
#ifdef USE_OPENGL
    QtGlAccelWidget *gl = (QtGlAccelWidget *)accel;
    if(!gl) return false;
    gl->makeCurrent();
    if(!gl->setImage(pic))
    {
        gl->doneCurrent();
        return false;
    }
    gl->update();
    gl->doneCurrent();
    return true;
#else
    return false;
#endif
}

/**
    \fn paintEvent( QPaintEvent *ev))
    \brief Repaint our "video" widget, ignore when accelRender is on
*/
void ADM_QCanvas::paintEvent(QPaintEvent *ev)
{
	if(!dataBuffer)
		return ;

	QImage image(dataBuffer,_w,_h,_l,QImage::Format_RGB32);
#if QT_VERSION >= QT_VERSION_CHECK(5,5,0)
	image.setDevicePixelRatio(devicePixelRatioF());
#endif
	QPainter painter(this);
	painter.drawImage(QPoint(0,0),image);
	painter.end();
}
//EOF
