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
    mode = (ADM_rubberBandFlags)(ADM_RUBBER_BAND_GRIPS_FIRST | ADM_RUBBER_BAND_GRIPS_SECOND);
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
    if(mode & ADM_RUBBER_BAND_GRIPS_FIRST)
        painter.fillPath(topLeft, solid);
    if(mode & ADM_RUBBER_BAND_GRIPS_SECOND)
        painter.fillPath(bottomRight, solid);

    painter.end();
}

/**
    \fn Ctor
*/
ADM_rubberControl::ADM_rubberControl(ADM_flyDialog *fly, QWidget *parent) : QWidget(parent)
{
#define FIRST_RUN_MARKER (-99)
    nestedIgnore = FIRST_RUN_MARKER;
    flyParent=fly;
    rubberControlParent = parent;
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
    grip1ptr = (void *)grip1;
    grip2ptr = (void *)grip2;
    drag = false;
}

/**
    \fn resizeEvent
*/
void ADM_rubberControl::resizeEvent(QResizeEvent *)
{
    rubberband->resize(size());
    if (nestedIgnore)
        return;
    flyParent->bandResized(pos().x(), pos().y(), size().width(), size().height());
}

/**
    \fn showEvent
    \brief Inhibit callback until the dialog has become visible.
*/
void ADM_rubberControl::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    if (nestedIgnore == FIRST_RUN_MARKER)
        nestedIgnore = 0;
}

/**
    \fn enterEvent
*/
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
void ADM_rubberControl::enterEvent(QEvent *event)
#else
void ADM_rubberControl::enterEvent(QEnterEvent *event)
#endif
{
    setCursor(Qt::SizeAllCursor);
}

/**
    \fn leaveEvent
*/
void ADM_rubberControl::leaveEvent(QEvent *event)
{
    setCursor(Qt::ArrowCursor);
}

/**
    \fn mousePressEvent
*/
void ADM_rubberControl::mousePressEvent(QMouseEvent *event)
{
    dragOffset =
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
    event->globalPos()
#else
    event->globalPosition().toPoint()
#endif
    - pos();
    dragGeometry = rect();
    drag = true;
}

/**
    \fn mouseReleaseEvent
*/
void ADM_rubberControl::mouseReleaseEvent(QMouseEvent *event)
{
    drag = false;
}

/**
    \fn mouseMoveEvent
*/
void ADM_rubberControl::mouseMoveEvent(QMouseEvent *event)
{
    if (drag)
    {
        int x, y, w, h, pw, ph;
        QPoint delta =
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
        event->globalPos()
#else
        event->globalPosition().toPoint()
#endif
        - dragOffset;
        x = delta.x();
        y = delta.y();
        w = dragGeometry.width();
        h = dragGeometry.height();
        pw = rubberControlParent->size().width();
        ph = rubberControlParent->size().height();
        if (x < 0) x = 0;
        if (y < 0) y = 0;
        if ((x+w) > pw) x = pw - w;
        if ((y+h) > ph) y = ph - h;
        // double check
        if (x < 0) x = 0;
        if (y < 0) y = 0;
        move(x,y);
        flyParent->bandMoved(x, y, w, h);
    }
}

/**
    \fn sizeGripEnable
*/
void ADM_rubberControl::sizeGripEnable(bool topLeftEnabled, bool bottomRightEnabled)
{
    transparentSizeGrip *g1 = (transparentSizeGrip *)grip1ptr;
    transparentSizeGrip *g2 = (transparentSizeGrip *)grip2ptr;
    g1->setEnabled(topLeftEnabled);
    g2->setEnabled(bottomRightEnabled);
    int flags = ADM_QRubberBand::ADM_RUBBER_BAND_GRIPS_NONE;
    if(topLeftEnabled)
        flags |= ADM_QRubberBand::ADM_RUBBER_BAND_GRIPS_FIRST;
    if(bottomRightEnabled)
        flags |= ADM_QRubberBand::ADM_RUBBER_BAND_GRIPS_SECOND;
    rubberband->drawGrips(flags);
    rubberband->update();
}
//EOF
