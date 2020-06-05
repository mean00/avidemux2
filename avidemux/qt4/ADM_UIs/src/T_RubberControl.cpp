/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************///

#include <QPainter>
#include <QRubberBand>
#include <QBoxLayout>
#include <QSizeGrip>

#if QT_VERSION >= QT_VERSION_CHECK(5,15,0)
    #include <QPainterPath>
#endif

#include "ADM_default.h"
#include "DIA_flyDialogQt4.h"

class transparentSizeGrip : public QSizeGrip
{
public:
    transparentSizeGrip(QWidget *parent) : QSizeGrip(parent) { };
    ~transparentSizeGrip() { };
private:
    void paintEvent(QPaintEvent *event);
};

/**
    \fn paintEvent
*/
void transparentSizeGrip::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setPen(Qt::NoPen);
    QColor color(Qt::black);
    color.setAlpha(0);
    QBrush invisible = QBrush(color, Qt::SolidPattern);
    painter.fillRect(rect(),invisible);
    painter.end();
}

/**

*/
ADM_QRubberBand::ADM_QRubberBand(QWidget *parent) : QRubberBand(QRubberBand::Rectangle, parent)
{
}

ADM_QRubberBand::~ADM_QRubberBand()
{
}

/**
    \fn paintEvent
    \brief Override platform-dependent appearance of QRubberBand
*/
void ADM_QRubberBand::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    QPen pen;
    pen.setWidth(2);
    QColor color(Qt::red);
    pen.setColor(color);
    painter.setPen(pen);
    QRect adjustedRect = rect().adjusted(1,1,-1,-1);
    painter.drawRect(adjustedRect);
    color.setAlpha(50);
    QBrush brush = QBrush(color, Qt::DiagCrossPattern);
    adjustedRect = adjustedRect.adjusted(1,1,-1,-1);
    painter.fillRect(adjustedRect, brush);

    QPainterPath topLeft;
    topLeft.moveTo(4,4);
    topLeft.lineTo(12,4);
    topLeft.lineTo(4,12);
    topLeft.lineTo(4,4);

    QPainterPath bottomRight;
    bottomRight.moveTo(width()-4,height()-4);
    bottomRight.lineTo(width()-12,height()-4);
    bottomRight.lineTo(width()-4,height()-12);
    bottomRight.lineTo(width()-4,height()-4);

    painter.setPen(Qt::NoPen);
    QBrush solid = QBrush(Qt::red, Qt::SolidPattern);
    painter.fillPath(topLeft, solid);
    painter.fillPath(bottomRight, solid);

    painter.end();
}

/**
    \fn Ctor
*/
ADM_rubberControl::ADM_rubberControl(ADM_flyDialog *fly, QWidget *parent) : QWidget(parent)
{
    nestedIgnore=0;
    flyParent=fly;
    // tell QSizeGrip to resize this widget instead of top-level window
    setWindowFlags(Qt::SubWindow);
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    transparentSizeGrip *grip1 = new transparentSizeGrip(this);
    transparentSizeGrip *grip2 = new transparentSizeGrip(this);
    grip1->setFixedSize(10,10);
    grip2->setFixedSize(10,10);
    layout->addWidget(grip1, 0, Qt::AlignLeft | Qt::AlignTop);
    layout->addWidget(grip2, 0, Qt::AlignRight | Qt::AlignBottom);
    rubberband = new ADM_QRubberBand(this);
}

/**
    \fn resizeEvent
*/
void ADM_rubberControl::resizeEvent(QResizeEvent *)
{
    int x, y, w, h;
    x = pos().x();
    y = pos().y();
    w = size().width();
    h = size().height();
    rubberband->resize(size());
    if(!nestedIgnore)
        flyParent->bandResized(x, y, w, h);
}
//EOF
