
/***************************************************************************
Custom slider
***************************************************************************/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include <QtGui/QPainter>
#include <QtGui/QSlider>

#include "ADM_qslider.h"

ADM_QSlider::ADM_QSlider(QWidget *parent) : QSlider(parent)
{
	frameCount = markerA = markerB = 0;
}

void ADM_QSlider::paintEvent(QPaintEvent *event)
{
	QSlider::paintEvent(event);	

	int a = markerA, b = markerB;

	if (markerA > markerB)
	{
		b = markerA;
		a = markerB; 
	}

	if (frameCount > 0 && (a != 0 || b != frameCount))
	{
		int left = (a * width()) / frameCount;
		int right = (b * width()) / frameCount;

		QPainter painter(this);

		painter.setPen(Qt::blue);
		painter.drawRect(left, 1, right - left, height() - 3);
		painter.end();
	}
}

void ADM_QSlider::setMarkerA(uint32_t frameIndex)
{
	setMarkers(frameIndex, markerB);
}

void ADM_QSlider::setMarkerB(uint32_t frameIndex)
{
	setMarkers(markerA, frameIndex);
}

void ADM_QSlider::setMarkers(uint32_t frameIndexA, uint32_t frameIndexB)
{
	if (frameIndexA > frameCount)
		printf("[ADM_QSlider] Marker A is out of bounds (%u, %u)\n", markerA, frameCount);
	else if (frameIndexB > frameCount)
		printf("[ADM_QSlider] Marker B is out of bounds (%u, %u)\n", markerB, frameCount);
	else
	{
		markerA = frameIndexA;
		markerB = frameIndexB;

		repaint();
	}
}

void ADM_QSlider::setFrameCount(uint32_t count)
{
	frameCount = count;
	repaint();
}

//EOF 
