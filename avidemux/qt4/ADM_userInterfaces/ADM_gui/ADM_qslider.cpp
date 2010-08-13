
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
#include <stdio.h>
#include <stdlib.h>
#include "ADM_inttype.h"
#include <QtGui/QPainter>
#include <QtGui/QSlider>
#include "ADM_qslider.h"
#include "ADM_assert.h"
/**
    \fn ADM_QSlider
*/

ADM_QSlider::ADM_QSlider(QWidget *parent) : QSlider(parent)
{
   totalDuration= markerATime= markerBTime =0;
	
}
/**
    \fn paintEvent
*/
void ADM_QSlider::paintEvent(QPaintEvent *event)
{
	QSlider::paintEvent(event);	

	uint64_t a = markerATime, b = markerBTime;

	if (markerATime > markerBTime)
	{
		b = markerATime;
		a = markerBTime; 
	}

	if (totalDuration > 0LL && (a != 0 || b != totalDuration))
	{
		int left  = (int)(((double)a * width()) / (double)totalDuration);
		int right = (int)(((double)b * width()) / (double)totalDuration);

		QPainter painter(this);

		painter.setPen(Qt::blue);
		painter.drawRect(left, 1, right - left, height() - 3);
		painter.end();
	}
}
/**
    \fn setMarkerA
*/

void ADM_QSlider::setMarkerA(uint64_t frameIndex)
{
	setMarkers(frameIndex, markerBTime);
}
/**
    \fn setMarkerB
*/
void ADM_QSlider::setMarkerB(uint64_t frameIndex)
{
	setMarkers(markerATime, frameIndex);
}
/**
    \fn setMarkers
*/
void ADM_QSlider::setMarkers(uint64_t frameIndexA, uint64_t frameIndexB)
{
	if (frameIndexA > totalDuration)
		printf("[ADM_QSlider] Marker A is out of bounds (%"LLU", %"LLU")\n", markerATime, totalDuration);
	else if (frameIndexB > totalDuration)
		printf("[ADM_QSlider] Marker B is out of bounds (%"LLU", %"LLU")\n", markerBTime, totalDuration);
	else
	{
		markerATime = frameIndexA;
		markerBTime = frameIndexB;

		repaint();
	}
}
/**
        \fn setTotalDuration
*/
void ADM_QSlider::setTotalDuration(uint64_t duration)
{
	totalDuration = duration;
	repaint();
}

//EOF 
