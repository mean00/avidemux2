
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

        setValue(newVal);
        e->accept();
    }else
    {
        QSlider::mousePressEvent(e);
    }
}
//EOF
