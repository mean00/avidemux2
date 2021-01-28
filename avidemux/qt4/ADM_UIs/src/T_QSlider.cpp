
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
            int adjPosX = e->x();
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
            int adjPosY = height() - e->y();
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
//EOF
