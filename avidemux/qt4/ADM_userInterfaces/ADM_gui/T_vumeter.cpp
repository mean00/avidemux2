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

#include <QFrame>
#include <QImage>
#include <QPainter>

/* Probably on unix/X11 ..*/
#ifdef __APPLE__
#include <Carbon/Carbon.h>
#elif !defined(_WIN32)
#if QT_VERSION < QT_VERSION_CHECK(5,0,0) 
#include <QX11Info>
#endif
#endif
#include "ADM_default.h"
#include "T_vumeter.h"

static ADM_Qvumeter *vuWidget = NULL;

//  #warning Assume rgb bbuffer is 32bits aligned...
#define GREEN   0xFF00FF00
#define DGREEN  0xFF00BF00
#define YELLOW  0xFFA0C000
#define DYELLOW 0xFF789000
#define RED     0xFFFF0000
#define DRED    0xFFBF0000
#define BLACK   0xFF000000

#define VU_BAR_WIDTH  5
#define VU_PEAK_WIDTH 7
#define VU_BOTTOM_SCALE -60.0	// dBFS, must be negative
#define VU_DECAY      1.0

ADM_Qvumeter::ADM_Qvumeter(QWidget *z, int width, int height) : QWidget(z)
{
    this->resize(width, height);
    z->resize(width + 2, height + 2);

    rgbDataBuffer = new uint8_t[width * height * 4];
    for(int i=0;i<88;i++)
    {
        uint8_t *ptr = rgbDataBuffer + (width * i * 4);
        uint32_t *data = (uint32_t *)ptr;
        for(int j=0;j<64;j++)
        {
            *data++=BLACK;
        }
    }
    for (int i=0; i<8; i++)
        peaks[i].maxPos = 0;
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
    vuWidget = new ADM_Qvumeter(host, 64, 88);
    vuWidget->show();

    return true;
}
/**
    \fn UI_vuUpdate
    \brief Update vumeter, input is volume per channel as integer dBFS (valid range -100 to +3), mark inactive channel with +255 (> 3)
*/
bool UI_vuUpdate(int32_t volume[8])
{
    int width = vuWidget->width();
    uint8_t * basePtr = vuWidget->rgbDataBuffer;
    
    int headroom,overdrive;
    float l;
    l = -18; // headroom -18 dBFS (Digital broadcasts and ordinary digital recordings) (<84 & >= 66)
    l -= VU_BOTTOM_SCALE;
    l /= (3.0 - VU_BOTTOM_SCALE);
    l *= 87.0;
    headroom = l+0.49;
    l = 0; // overdrive >= 0 dBFS
    l -= VU_BOTTOM_SCALE;
    l /= (3.0 - VU_BOTTOM_SCALE);
    l *= 87.0;
    overdrive = l+0.49;
    
    int activeChannelCount = 0;
    int vol[8], peak[8];
    uint32_t peakColor[8];
    for(int i=0;i<8;i++)
    {
        if (volume[i] <= 3)
        {
            //Decay
            if (vuWidget->peaks[i].maxPos > 0)
            {
                if (vuWidget->peaks[i].maxPos > VU_DECAY)
                    vuWidget->peaks[i].maxPos -= VU_DECAY;
                else
                    vuWidget->peaks[i].maxPos = 0;
            }
            // Map volume to display
            // +3 -> 87
            // VU_BOTTOM_SCALE -> 0
            float v = volume[i];
            v -= VU_BOTTOM_SCALE;
            v /= (3.0 - VU_BOTTOM_SCALE);
            v *= 87.0;
            vol[activeChannelCount] = v+0.49;
            if (vol[activeChannelCount] > 87)
                vol[activeChannelCount] = 87;
            if (vol[activeChannelCount] < 0)
                vol[activeChannelCount] = 0;
            if (vol[activeChannelCount] > vuWidget->peaks[i].maxPos)
            {
                vuWidget->peaks[i].maxPos = vol[activeChannelCount];
                if(vol[activeChannelCount]<headroom) vuWidget->peaks[i].maxColor = GREEN;
                else if(vol[activeChannelCount]<overdrive) vuWidget->peaks[i].maxColor = YELLOW;
                else vuWidget->peaks[i].maxColor = RED;
            }
            if (vol[activeChannelCount] < 1)
                vol[activeChannelCount] = 1;
            peak[activeChannelCount] = vuWidget->peaks[i].maxPos;
            peakColor[activeChannelCount] = vuWidget->peaks[i].maxColor;

            activeChannelCount++;
        }
        else
        {
            vuWidget->peaks[i].maxPos = 0;
        }
    }
    
    if (activeChannelCount == 0)	// just clear
    {
        for(int i=0;i<88;i++)
        {
            uint8_t *ptr = basePtr + (width * i * 4);
            uint32_t *data = (uint32_t *)ptr;
            for(int j=0;j<64;j++)
            {
                *data++=BLACK;
            }
        }
        vuWidget->update();
        return true;
    }

    // Draw
    for(int i=0;i<88;i++)
    {
        uint8_t *ptr = basePtr + (width * i * 4);
        uint32_t *data = (uint32_t *)ptr;
        int h = 87-i;
        for(int j=0;j<64;j++)
        {
            *data=BLACK;
            int w = 64/activeChannelCount;
            int v = j/w;
            int x = j%w;
            if (v >= activeChannelCount)
            {
                v = activeChannelCount - 1;
                x = 0;
            }
            if (peak[v] == h)
            {
                if ((x >= (9-activeChannelCount)) && (x < (w-(9-activeChannelCount))))
                    *data = peakColor[v];
            } else {
                if (vol[v] >= h)
                    if ((x >= (10-activeChannelCount)) && (x < (w-(10-activeChannelCount))))
                    {
                        if(h<headroom) *data=(i%4==3)?DGREEN:GREEN;
                        else if(h<overdrive) *data=(i%4==3)?DYELLOW:YELLOW;
                        else *data=(i%4==3)?DRED:RED;
                    }
            }
            data++;
        }
    }

#if 0
      ADM_info("VU : LEFT %"PRIu32" CENTER %"PRIu32" RIGHT %"PRIu32" REARLEFT %"PRIu32" REARRIGHT %"PRIu32"\n",
                    volume[0],volume[1],volume[2],volume[3],volume[4]);
#endif
    vuWidget->update();
    return true;
}
//EOF
