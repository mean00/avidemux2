
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
#include <QSlider>
#include <QStyleOptionSlider>
#include <QToolTip>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include "ADM_toolkitQt.h"
#include "ADM_assert.h"
/**
    \fn ADM_QSlider
*/

ADM_QSlider::ADM_QSlider(QWidget *parent) : QSlider(parent)
{
}

/*
    \fn mousePressEvent
    \brief seek straight to the mouse position, code by https://stackoverflow.com/users/1777490/bitok
    \      see https://stackoverflow.com/questions/11132597/qslider-mouse-direct-jump
*/
void ADM_QSlider::mousePressEvent(QMouseEvent *e)
{
    QStyleOptionSlider opt;
    initStyleOption(&opt);
    QRect sr = style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderHandle, this);

    if (e->button() == Qt::LeftButton && !sr.contains(e->pos()))
    {
        int newVal;
        if (orientation() == Qt::Horizontal)
        {
            double halfHandleWidth = (0.5 * sr.width()) + 0.5;
            int adjPosX =
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
            e->x();
#else
            e->position().x();
#endif
            if (adjPosX < halfHandleWidth)
                adjPosX = halfHandleWidth;
            if (adjPosX > width() - halfHandleWidth)
                adjPosX = width() - halfHandleWidth;
            double newWidth = width() - (halfHandleWidth * 2);
            double normalizedPosition = (adjPosX - halfHandleWidth) / newWidth;

            newVal = minimum() + ((maximum() - minimum()) * normalizedPosition);
        }else
        {
            double halfHandleHeight = (0.5 * sr.height()) + 0.5;
            int adjPosY = height() -
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
            e->y();
#else
            e->position().y();
#endif
            if (adjPosY < halfHandleHeight)
                adjPosY = halfHandleHeight;
            if (adjPosY > height() - halfHandleHeight)
                adjPosY = height() - halfHandleHeight;
            double newHeight = height() - (halfHandleHeight * 2);
            double normalizedPosition = (adjPosY - halfHandleHeight) / newHeight;

            newVal = minimum() + (maximum() - minimum()) * normalizedPosition;
        }
        if(layoutDirection() == Qt::LeftToRight)
            setValue(newVal);
        else
            setValue(maximum() - newVal);
        e->accept();
    }else
    {
        QSlider::mousePressEvent(e);
    }
}

/**
    \fn ADM_SliderIndicator
*/
ADM_SliderIndicator::ADM_SliderIndicator(QWidget *parent) : QSlider(parent)
{
    _scaleNum = _scaleDen = 1;
    _precision = 0;
}

/**
    \fn setScale
*/
void ADM_SliderIndicator::setScale(int num, int den, int precision)
{
    if(num > 0 && den > 0)
    {
        _scaleNum = num;
        _scaleDen = den;
    }
    if(precision >= 0)
        _precision = precision;
    if(_precision > 3)
        _precision = 3; // max 3 digits after decimal point
}

/**
    \fn sliderChange
    \brief show slider value in a tooltip below slider handle on drag or move
           stolen from https://stackoverflow.com/questions/18383885/qslider-show-min-max-and-current-value
*/
void ADM_SliderIndicator::sliderChange(QAbstractSlider::SliderChange change)
{
    QSlider::sliderChange(change);

    if(change == QAbstractSlider::SliderValueChange)
    {
        QStyleOptionSlider opt;
        initStyleOption(&opt);

        QRect sr = style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderHandle, this);
        QPoint bottomLeftCorner = sr.bottomLeft();
        QPoint bottomRightCorner = sr.bottomRight();
        int xpos = bottomLeftCorner.x() + bottomRightCorner.x();
        int ypos = bottomLeftCorner.y();

        QString text;
        if(_scaleDen > 1)
        {
            double dv = value();
            dv *= _scaleNum;
            dv /= _scaleDen;
            if(_precision)
                text = QString::number(dv,'f',_precision);
            else
                text = QString::number((int)(dv + 0.49));
        }else
        {
            text = QString::number(_scaleNum * value()); // precision is ignored
        }

        QFontMetrics fm = fontMetrics();
        xpos -= fm.boundingRect(text).width() + 12;
        xpos /= 2;

        QToolTip::showText(mapToGlobal(QPoint(xpos, ypos)), text, this);
    }
}


ADM_flyNavSlider::ADM_flyNavSlider(QWidget *parent) : ADM_QSlider(parent)
{
    _invertWheel = false;
    totalDuration = markerATime = markerBTime = 0;
}

void ADM_flyNavSlider::wheelEvent(QWheelEvent *e)
{
    int delta;
#if QT_VERSION < QT_VERSION_CHECK(5,0,0)
    //printf("Wheel : %d\n",e->delta());
    delta = e->delta();
#else
    delta = e->angleDelta().ry();
#endif   
    if (_invertWheel)
        delta *= -1;
    
    if (delta > 0)
        this->triggerAction(QAbstractSlider::SliderSingleStepAdd);
    else
    if (delta < 0)
        this->triggerAction(QAbstractSlider::SliderSingleStepSub);
    
    e->accept();
}

void ADM_flyNavSlider::setInvertedWheel(bool inverted)
{
    _invertWheel = inverted;
    totalDuration= markerATime= markerBTime =0;
}

void ADM_flyNavSlider::setMarkers(uint64_t totalDuration, uint64_t markerATime, uint64_t markerBTime)
{
    this->totalDuration = totalDuration;
    this->markerATime = markerATime;
    this->markerBTime = markerBTime;
}

/**
    \fn drawSelection
*/
void ADM_flyNavSlider::drawSelection(void)
{
    if (!totalDuration) return;

    uint64_t a = markerATime, b = markerBTime;

    if (markerATime > markerBTime)
    {
        b = markerATime;
        a = markerBTime;
    }

    if (a == 0 && b >= totalDuration) return;

    int left  = (int)(((double)a * width()) / (double)totalDuration);
    int right = (int)(((double)b * width()) / (double)totalDuration);

    if (left < 1) left = 1;
    if (right < 1) right = 1;
    if (left > width() - 1) left = width() - 1;
    if (right > width() - 1) right = width() - 1;

    QPainter painter(this);
    if (this->isDarkMode())
        painter.setPen(QColor(30,144,255));
    else
        painter.setPen(Qt::blue);

    const int r = 6; // curvature radius
    const int d = r * 2;
    const int h = height() - 2;
    const int span = right - left;

    if (layoutDirection() == Qt::RightToLeft)
    { // unswap markers' x coordinates
        int swp = width() - right;
        right = width() - left;
        left = swp;
    }
    // Shall we hint at start or end marker being at its initial position?
    if ((span > r) && (h > d))
    {
        if (a > 0 && b == totalDuration)
        {
            if (layoutDirection() == Qt::LeftToRight)
            {
                QPainterPath sel(QPointF(left, 1)); // starting top left
                sel.lineTo(right - r, 1);
                sel.arcTo(right - d, 1, d, d, 90, -90); // top right, rounded
                sel.lineTo(right, h - r);
                sel.arcTo(right - d, h - d, d, d, 0, -90); // bottom right, rounded
                sel.lineTo(left, h); // bottom left
                sel.closeSubpath();
                painter.drawPath(sel);
            } else
            {
                QPainterPath sel(QPointF(left + r, 1)); // starting past left rounded corner
                sel.lineTo(right, 1); // top right
                sel.lineTo(right, h); // bottom right
                sel.lineTo(left + r, h);
                sel.arcTo(left, h - d, d, d, 270, -90); // bottom left rounded corner
                sel.lineTo(left, r + 1);
                sel.arcTo(left, 1, d, d, 180, -90); // top left rounded corner
                painter.drawPath(sel);
            }
            return;
        }
        if (a <= 0 && b < totalDuration)
        {
            if (layoutDirection() == Qt::LeftToRight)
            {
                QPainterPath sel(QPointF(left + r, 1)); // starting past left rounded corner
                sel.lineTo(right, 1); // top right
                sel.lineTo(right, h); // bottom right
                sel.lineTo(left + r, h);
                sel.arcTo(left, h - d, d, d, 270, -90); // bottom left, rounded
                sel.lineTo(left, 1 + r);
                sel.arcTo(left, 1, d, d, 180, -90); // top left, rounded
                painter.drawPath(sel);
            } else
            {
                QPainterPath sel(QPointF(left, 1)); // starting top left
                sel.lineTo(right - r, 1);
                sel.arcTo(right - d, 1, d, d, 90, -90); // top right rounded corner
                sel.lineTo(right, h - r);
                sel.arcTo(right - d, h - d, d, d, 0, -90); // bottom right rounded corner
                sel.lineTo(left, h); // bottom left
                sel.closeSubpath();
                painter.drawPath(sel);
            }
            return;
        }
    }
    // No, draw a simple rectangle. We need to reduce the height by one, why?
    painter.drawRect(left, 1, span, h - 1);
}

/**
    \fn paintEvent
*/
void ADM_flyNavSlider::paintEvent(QPaintEvent *event)
{
    /* TODO
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
    }*/

    QSlider::paintEvent(event);

    drawSelection();
}

/**
    \fn isDarkMode
*/
bool ADM_flyNavSlider::isDarkMode()
{
    QColor bgColor = this->palette().color(QPalette::Window);
    return (bgColor.value() < 128);
}

//EOF
