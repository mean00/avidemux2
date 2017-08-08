
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
class ADM_mwNavSlider : public ADM_QSlider
{
    Q_OBJECT
protected:
    uint64_t totalDuration, markerATime, markerBTime;
    void paintEvent(QPaintEvent *event);

public:
    ADM_mwNavSlider(QWidget *parent);

    void setMarkerA(uint64_t frameIndex);
    void setMarkerB(uint64_t frameIndex);
    void setMarkers(uint64_t frameIndexA, uint64_t frameIndexB);
    void setTotalDuration(uint64_t duration);
    void wheelEvent(QWheelEvent *e);
signals:
    void sliderAction(int value);
};

