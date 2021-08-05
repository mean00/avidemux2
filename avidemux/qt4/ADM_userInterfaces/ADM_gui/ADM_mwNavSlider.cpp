
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
#include <QPainter>
#include <QSlider>
#include <QWheelEvent>
#include "ADM_mwNavSlider.h"
#include "ADM_assert.h"
#include "ADM_default.h"
/**
    \fn ADM_QSlider
*/
ADM_mwNavSlider::ADM_mwNavSlider(QWidget *parent) : ADM_QSlider(parent)
{
    totalDuration= markerATime= markerBTime =0;
    segments = NULL;
    numOfSegments = 0;
}
/**
    \fn dtor
*/
ADM_mwNavSlider::~ADM_mwNavSlider()
{
    if (segments)
        delete [] segments;
    segments = NULL;
}
/**
    \fn paintEvent
*/
void ADM_mwNavSlider::paintEvent(QPaintEvent *event)
{
    if (segments && (numOfSegments > 0) && (totalDuration > 0LL))
    {
        int pos, prevpos;
        prevpos = -1;
        QPainter painter(this);
        for (uint32_t i=0; i<numOfSegments; i++)
        {
            if (i==0)    // do not draw at the start position
                continue;
            if (segments[i] == ADM_NO_PTS)
                continue;
            pos = (int)(((double)segments[i] * width()) / (double)totalDuration);
            if(pos < 1) pos = 1;
            if(pos > width() - 1) pos = width() - 1;
            if (pos == prevpos)
                painter.setPen(Qt::darkMagenta);
            else
                painter.setPen(Qt::red);
            prevpos = pos;

            if(layoutDirection() == Qt::LeftToRight)
                painter.drawLine(pos, 1, pos, height() - 3);
            else
                painter.drawLine(width() - pos, 1, width() - pos, height() - 3);
        }
        painter.end();
    }

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

        if(left < 1) left = 1;
        if(right < 1) right = 1;
        if(left > width() - 1) left = width() - 1;
        if(right > width() - 1) right = width() - 1;

        QPainter painter(this);
        painter.setPen(Qt::blue);
        if(layoutDirection() == Qt::LeftToRight)
            painter.drawRect(left, 1, right - left, height() - 3);
        else
            painter.drawRect(width() - right, 1, right - left, height() - 3);
        painter.end();
    }
}
/**
    \fn setMarkerA
*/
void ADM_mwNavSlider::setMarkerA(uint64_t frameIndex)
{
    setMarkers(frameIndex, markerBTime);
}
/**
    \fn setMarkerB
*/
void ADM_mwNavSlider::setMarkerB(uint64_t frameIndex)
{
    setMarkers(markerATime, frameIndex);
}
/**
    \fn setMarkers
*/
void ADM_mwNavSlider::setMarkers(uint64_t frameIndexA, uint64_t frameIndexB)
{
    if (frameIndexA > totalDuration)
        printf("[ADM_QSlider] Marker A is out of bounds (%" PRIu64", %" PRIu64")\n", markerATime, totalDuration);
    else if (frameIndexB > totalDuration)
        printf("[ADM_QSlider] Marker B is out of bounds (%" PRIu64", %" PRIu64")\n", markerBTime, totalDuration);
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
void ADM_mwNavSlider::setTotalDuration(uint64_t duration)
{
    totalDuration = duration;
    repaint();
}
/**
    \fn setSegments
*/
void ADM_mwNavSlider::setSegments(uint32_t numOfSegs, uint64_t * segPts)
{
    if (segments)
        delete [] segments;
    segments = new uint64_t[numOfSegs];
    for (uint32_t i=0; i<numOfSegs; i++)
        segments[i] = segPts[i];
    numOfSegments = numOfSegs;
}
/*
    \fn wheelEvent
 */
void ADM_mwNavSlider::wheelEvent(QWheelEvent *e)
{
#if QT_VERSION < QT_VERSION_CHECK(5,0,0)
    //printf("Wheel : %d\n",e->delta());
    emit sliderAction(e->delta());
#else
    emit sliderAction(e->angleDelta().ry());
#endif
}
//EOF
