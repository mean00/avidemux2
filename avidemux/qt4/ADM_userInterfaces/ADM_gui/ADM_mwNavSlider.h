
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

#pragma once
#include "ADM_toolkitQt.h"

/**
    \class ADM_mwNavSlider
*/
class ADM_mwNavSlider : public ADM_flyNavSlider
{
    Q_OBJECT
protected:
    uint64_t * segments;
    uint32_t numOfSegments;

    void drawCutPoints(void);
    void paintEvent(QPaintEvent *event);
    void wheelEvent(QWheelEvent *e);

public:
    ADM_mwNavSlider(QWidget *parent);
    virtual ~ADM_mwNavSlider();

    void setMarkerA(uint64_t frameIndex);
    void setMarkerB(uint64_t frameIndex);
    void setMarkers(uint64_t frameIndexA, uint64_t frameIndexB);
    void setTotalDuration(uint64_t duration);
    void setSegments(uint32_t numOfSegs, uint64_t * segPts);
signals:
    void sliderAction(int value);
};

