/***************************************************************************
    \file  T_thumbslider
    \brief Manage thumbslider/navigator widget
    \author JM
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef T_THUMBSLIDER_H
#define T_THUMBSLIDER_H

#include <QtGui/QPainter>
#include <QtGui/QPaintEvent>
#include <QtGui/QMouseEvent>
#include <QtGui/QSlider>

class ThumbSlider : public QAbstractSlider
{
	Q_OBJECT

private:
	int timerId, count, lock;

	void drawBackground(QPainter *painter);
	void drawLines(QPainter *painter);
	void drawBorders(QPainter *painter);
	void drawEdges(QPainter *painter);

public:
	ThumbSlider(QWidget *parent = 0);

protected:
	void timerEvent(QTimerEvent *event);
	void paintEvent(QPaintEvent *event);
	void mousePressEvent(QMouseEvent *event);
	void mouseMoveEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);

signals:
	void valueEmitted(int value);
};

#endif
