#ifndef Q_resizing_h
#define Q_resizing_h

#include "ui_resizing.h"

typedef struct resParam
{
	uint32_t width,height;
	uint32_t originalWidth, originalHeight;
	uint32_t fps1000;
	uint32_t algo;
	uint32_t pal;
} resParam;

class resizeWindow : public QDialog
{
	Q_OBJECT

private:
	int lastPercentage;
	void updateWidthHeightSpinners(bool useHeightAsRef = false);
	void updateSlider();
	void connectDimensionControls();
	void disconnectDimensionControls();
	void roundUp(int xx, int yy);

protected:
	resParam *_param;

public:
	resizeWindow(resParam *param);
	Ui_resizeDialog ui;

public slots:
	void gather(void);
	void okButtonClicked();
	void sliderChanged(int value);
	void percentageSpinBoxChanged(int value);
	void widthSpinBoxChanged(int value);
	void heightSpinBoxChanged(int value);
	void lockArToggled(bool toggled);
	void roundupToggled(bool toggled);
	void aspectRatioChanged(int index);
};
#endif	// Q_resizing_h
