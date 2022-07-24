/***************************************************************************
    copyright            : (C) 2011 by mean
    email                : fixounet@free.fr
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#pragma once
#include "logo.h"

class draggableFrame : public QWidget
{
public:
    float       opacity;
    bool        setImage(ADMImage *pic);
                draggableFrame(ADM_flyDialog *fly, QWidget *parent);
    virtual     ~draggableFrame();

private:
    ADM_flyDialog *flyParent;
    bool        drag;
    QPoint      dragOffset;
    QRect       dragGeometry;
    uint8_t     *rgbdata;
    int         pitch;

    void        paintEvent(QPaintEvent *event);
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
    void        enterEvent(QEvent *event);
#else
    void        enterEvent(QEnterEvent *event);
#endif
    void        leaveEvent(QEvent *event);
    void        calculatePosition(QMouseEvent *event, int &xpos, int &ypos);
    void        mousePressEvent(QMouseEvent *event);
    void        mouseReleaseEvent(QMouseEvent *event);
    void        mouseMoveEvent(QMouseEvent *event);
};

/**
    \class flyLogo
*/
class flyLogo : public ADM_flyDialogYuv
{
public:
    logo        param;
    int         imageWidth, imageHeight;

    uint8_t     processYuv(ADMImage* in, ADMImage *out);
    uint8_t     download(void);
    uint8_t     upload(void) { return upload(true); }
    uint8_t     upload(bool toDraggableFrame);
                flyLogo (QDialog *parent, uint32_t width, uint32_t height, ADM_coreVideoFilter *in,
                                    ADM_QCanvas *canvas, ADM_flyNavSlider *slider);
    virtual     ~flyLogo();

    bool        bandMoved(int x, int y, int w, int h);
    bool        setXy(int x, int y);
    void        setTabOrder(void);
    void        adjustFrame(ADMImage *pic = NULL);
    void        updateFrameOpacity(void);
private:
    draggableFrame *frame;
    uint64_t    startOffset;
    uint64_t    endOffset;
};
// EOF

