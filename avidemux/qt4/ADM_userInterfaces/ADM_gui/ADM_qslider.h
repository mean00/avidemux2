#ifndef ADM_Q_SLIDER_H
#define ADM_Q_SLIDER_H
#include "ADM_inttype.h"
/**
    \class ADM_QSlider

*/
class ADM_QSlider : public QSlider
{
protected:
	uint64_t totalDuration, markerATime, markerBTime;
	void paintEvent(QPaintEvent *event);

public:
	ADM_QSlider(QWidget *parent = 0); 

	void setMarkerA(uint64_t frameIndex);
	void setMarkerB(uint64_t frameIndex);
	void setMarkers(uint64_t frameIndexA, uint64_t frameIndexB);
	void setTotalDuration(uint64_t duration);
};

#endif
