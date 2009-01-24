#ifndef ADM_Q_SLIDER_H
#define ADM_Q_SLIDER_H

class ADM_QSlider : public QSlider
{
protected:
	uint32_t frameCount, markerA, markerB;
	void paintEvent(QPaintEvent *event);

public:
	ADM_QSlider(QWidget *parent = 0); 

	void setMarkerA(uint32_t frameIndex);
	void setMarkerB(uint32_t frameIndex);
	void setMarkers(uint32_t frameIndexA, uint32_t frameIndexB);
	void setFrameCount(uint32_t count);
};

#endif
