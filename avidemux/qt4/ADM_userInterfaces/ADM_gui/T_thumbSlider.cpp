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
#include "T_thumbSlider.h"

#include <QStyle>

ThumbSlider::ThumbSlider(QWidget *parent) : QAbstractSlider(parent)
{
	setMaximum(100);
	setMinimum(-100);
	setValue(0);

	count = lock = 0;
}

void ThumbSlider::timerEvent(QTimerEvent *event)
{
	static const int jogScale[10] = {20, 15, 11, 8, 6, 5, 4, 3, 2, 0};
	int r = abs(value()) / 10;

	if (!r)
		return;
	if (lock)
		return;

	if (count)
		count--;

	if (count)
		return;

	count = (r > 9 ? jogScale[9] : jogScale[r]);


	lock++;

	emit valueEmitted(value());

	lock--;
}

void ThumbSlider::paintEvent(QPaintEvent *event)
{
	QPainter painter;

	painter.begin(this);
	painter.setClipRect(event->rect());
	painter.setPen(Qt::NoPen);

	drawBackground(&painter);
	drawLines(&painter);
	drawBorders(&painter);
	drawEdges(&painter);
}

void ThumbSlider::mousePressEvent(QMouseEvent *event)
{
	if (event->buttons() && Qt::LeftButton)
	{
		int value = QStyle::sliderValueFromPosition(minimum(), maximum(), event->x(), width(), false);

		setSliderPosition(value);
		timerId = startTimer(20);
		triggerAction(SliderMove);
	}
}

void ThumbSlider::mouseMoveEvent(QMouseEvent *event)
{
	if (event->buttons() && Qt::LeftButton)
	{
		int value = QStyle::sliderValueFromPosition(minimum(), maximum(), event->x(), width(), false);

		setSliderPosition(value);
		triggerAction(SliderMove);
	}
}

void ThumbSlider::mouseReleaseEvent(QMouseEvent *event)
{
	killTimer(timerId);
	setValue(0);
	count = 0;

	triggerAction(SliderNoAction);
}

void ThumbSlider::drawBackground(QPainter *painter)
{
	int middle = width() / 2;
	QLinearGradient base1 = QLinearGradient(0, 1, middle - 5, 1);
	QLinearGradient base2 = QLinearGradient(width() - 1, 1, middle + 5, 1);
	QColor color1, color2;

	color1.setRgbF(0.33, 0.33, 0.33);
	color2.setRgbF(0.88, 0.88, 0.88);

	base1.setColorAt(0, color1);
	base1.setColorAt(1, color2);
	base2.setColorAt(0, color1);
	base2.setColorAt(1, color2);

	painter->setBrush(base1);
	painter->drawRect(1, 0, middle - 1, height() - 1);

	painter->setBrush(base2);
	painter->drawRect(middle, 0, middle - 1, height() - 1);

	painter->setBrush(Qt::NoBrush);
}

void ThumbSlider::drawLines(QPainter *painter)
{
	float offset = (float)width() / 6.0;
	int pos = QStyle::sliderPositionFromValue(minimum(), maximum(), value(), width(), false);
	int middle = width() / 2;
	int start = pos - (width() / 2);
	float colourF;
	QPen pen;
	QColor color;

	pen.setCapStyle(Qt::FlatCap);

	for (int i = 0; i < 6; i++)
	{
		if (start > width())
			start -= width();
		else if (start < 0)
			start = width() + start;

		if (start <= middle)
			colourF = (float)start / (float)middle * 0.85;
		else
			colourF = ((float)(width() - start)) / (float)middle * 0.85;

		if (i == 3)
		{
			color.setRgbF(colourF, 0, 0);
			pen.setWidth(2);
		}
		else
		{
			color.setRgbF(colourF, colourF, colourF);
			pen.setWidth(1);
		}

		pen.setColor(color);
		painter->setPen(pen);
		painter->drawLine(start, 2, start, height() - 2);

		start += offset;
	}

	painter->setPen(Qt::NoPen);
}

void ThumbSlider::drawBorders(QPainter *painter)
{
	int middle = width() / 2;
	QColor color;

	color.setRgbF(0.53, 0.53, 0.53);

	painter->setPen(QPen(QBrush(color), 1, Qt::SolidLine, Qt::FlatCap));
	painter->drawLine(1, 0, width() - 1, 0);
	painter->drawLine(1, height() - 1, width() - 1, height() - 1);
	painter->drawLine(0, 1, 0, height() - 1);
	painter->drawLine(width() - 1, 1, width() - 1, height() - 1);

	QLinearGradient grad1 = QLinearGradient(0, 1, middle - 6, 1);
	QLinearGradient grad2 = QLinearGradient(width() - 1, 1, middle + 6, 1);
	QLinearGradient grad3 = QLinearGradient(0, height() - 2, middle - 6, height() - 2);
	QLinearGradient grad4 = QLinearGradient(width() - 1, height() - 2, middle + 6, height() - 2);
	QColor color1, color2;

	color1.setRgbF(0.00, 0.00, 0.00);
	color2.setRgbF(0.98, 0.98, 0.98);

	grad1.setColorAt(0, color1);
	grad1.setColorAt(1, color2);

	grad2.setColorAt(0, color1);
	grad2.setColorAt(1, color2);

	grad3.setColorAt(1, color2);
	grad3.setColorAt(0, color1);

	grad4.setColorAt(1, color2);
	grad4.setColorAt(0, color1);

	painter->setPen(QPen(grad1, 1, Qt::SolidLine, Qt::FlatCap));
	painter->drawLine(1, 1, middle, 1);
	painter->setPen(QPen(grad3, 1, Qt::SolidLine, Qt::FlatCap));
	painter->drawLine(1, height() - 2, middle, height() - 2);
	painter->setPen(QPen(grad2, 1, Qt::SolidLine, Qt::FlatCap));
	painter->drawLine(middle, 1, width() - 1, 1);
	painter->setPen(QPen(grad4, 1, Qt::SolidLine, Qt::FlatCap));
	painter->drawLine(middle, height() - 2, width() - 1, height() - 2);

	painter->setPen(Qt::NoPen);
}

void ThumbSlider::drawEdges(QPainter *painter)
{
	QLinearGradient grad1 = QLinearGradient(0, 2, 6, 2);
	QLinearGradient grad2 = QLinearGradient(width() - 1, 2, width() - 6, 2);
	QColor color1, color2;

	color1.setRgbF(0.00, 0.00, 0.00);
	color2.setRgbF(0.23, 0.23, 0.23);

	grad1.setColorAt(0, color1);
	grad1.setColorAt(1, color2);

	grad2.setColorAt(0, color1);
	grad2.setColorAt(1, color2);

	painter->setBrush(grad1);
	painter->drawRect(1, 2, 6, height() - 4);

	painter->setBrush(grad2);
	painter->drawRect(width() - 7, 2, 6, height() - 4);

	painter->setBrush(Qt::NoBrush);
}
