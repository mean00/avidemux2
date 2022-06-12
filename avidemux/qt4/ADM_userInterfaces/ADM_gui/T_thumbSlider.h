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

#include <QPainter>
#include <QPaintEvent>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QSlider>
#include <QToolButton>

class ThumbSlider : public QAbstractSlider
{
	Q_OBJECT

private:
	int timerId, count, lock, pos, stopping;
        QToolButton * resChgBtn;

	void stop(void);
	void drawBackground(QPainter *painter);
	void drawLines(QPainter *painter);
	void drawBorders(QPainter *painter);
	void drawEdges(QPainter *painter);

public:
	ThumbSlider(QWidget *parent, QToolButton * resolutionChangeBtn);
	void reset(void);

protected:
	void timerEvent(QTimerEvent *event);
	void paintEvent(QPaintEvent *event);
	void mousePressEvent(QMouseEvent *event);
	void mouseMoveEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);
	void wheelEvent(QWheelEvent *event);

signals:
	void valueEmitted(int value);
};

#endif
