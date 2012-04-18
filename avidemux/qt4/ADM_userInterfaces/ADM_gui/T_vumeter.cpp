/***************************************************************************
    \file T_vumeter.cpp
    \brief Class to draw volume unit audio graphs
    \author mean (c) 2009

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QtGui/QFrame>
#include <QtGui/QImage>
#include <QtGui/QPainter>

/* Probably on unix/X11 ..*/
#ifdef __APPLE__
#include <Carbon/Carbon.h>
#elif !defined(__WIN32)
#include <QtGui/QX11Info>
#endif
#include "ADM_default.h"
#include "T_vumeter.h"

static ADM_Qvumeter *vuWidget = NULL;

ADM_Qvumeter::ADM_Qvumeter(QWidget *z, int width, int height) : QWidget(z)
{
	this->resize(width, height);
	z->resize(width + 2, height + 2);

	rgbDataBuffer = new uint8_t[width * height * 4];
	memset(rgbDataBuffer, 0, width * height * 4);
}

ADM_Qvumeter::~ADM_Qvumeter()
{
	delete [] rgbDataBuffer;
}

void ADM_Qvumeter::paintEvent(QPaintEvent *ev)
{
	QImage image(this->rgbDataBuffer, this->width(), this->height(), QImage::Format_RGB32);
	QPainter painter(this);
	painter.drawImage(QPoint(1, 1), image);
	painter.end();
}

/**
    \fn UI_InitVUMeter
*/
bool UI_InitVUMeter(QFrame *host)
{
	vuWidget = new ADM_Qvumeter(host, 64, 64);
	vuWidget->show();

	return true;
}
/**
    \fn UI_vuUpdate
    \brief Update vumeter, input is volume per channel between 0 & 255
*/
  #warning Assume rgb bbuffer is 32bits aligned...
#define GREEN  0x0000FF00
#define YELLOW 0x00A0C000
#define RED    0x00FF0000

bool UI_vuUpdate(uint32_t volume[6])
{
	  memset(vuWidget->rgbDataBuffer, 0, vuWidget->width() * vuWidget->height() * 4);
      // Draw lines
      for(int i=0;i<6;i++)
      {
            int vol=volume[i];
            if(vol>63) vol=63;
            if(vol<3) vol=3;
			uint8_t *ptr = vuWidget->rgbDataBuffer + vuWidget->width() * (2 + 10 * i) * 4;
            uint32_t *data=(uint32_t *)(ptr);
            for(int j=0;j<vol;j++)
            {
                if(j<32) *data++=GREEN;
                else
                if(j<50) *data++=YELLOW;
                else *data++=RED;
            }
            for(int j=vol;j<64;j++)
            {
                *data++=0;
            }
      }
#if 0
      ADM_info("VU : LEFT %"LU" CENTER %"LU" RIGHT %"LU" REARLEFT %"LU" REARRIGHT %"LU"\n",
                    volume[0],volume[1],volume[2],volume[3],volume[4]);
#endif
      vuWidget->repaint();
      return true;
}
//EOF
