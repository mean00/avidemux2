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

#include <QtGui/QPainter>
#include <QtGui/QGraphicsView>
#include <QtGui/QSlider>

#include "ADM_default.h"

#include "ADM_videoFilter.h"
#include "DIA_flyDialogQt4.h"

void ADM_QCanvas::changeSize(uint32_t w,uint32_t h)
{
	_w=w;
	_h=h;
	dataBuffer=NULL;
	resize(w,h);
}

ADM_QCanvas::ADM_QCanvas(QWidget *z,uint32_t w,uint32_t h) : QWidget(z) 
{
	_w=w;
	_h=h;
	dataBuffer=NULL;
	resize(w,h);
}

ADM_QCanvas::~ADM_QCanvas() 
{
}

/**
    \fn paintEvent( QPaintEvent *ev))
    \brief Repaint our "video" widget, ignore when accelRender is on
*/
void ADM_QCanvas::paintEvent(QPaintEvent *ev)
{
	if(!dataBuffer)
		return ;

	QImage image(dataBuffer,_w,_h,QImage::Format_RGB32);
	QPainter painter(this);
	painter.drawImage(QPoint(0,0),image);
	painter.end();
}
//EOF
