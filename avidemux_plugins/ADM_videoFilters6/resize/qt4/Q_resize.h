#ifndef Q_resizing_h
#define Q_resizing_h

#include "ui_resizing.h"
#include "ADM_default.h"
#include "swresize.h"

typedef struct resParam
{
	uint32_t originalWidth, originalHeight;
	uint32_t fps1000;
    uint32_t pal;
    swresize rsz;
	
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
	resizeWindow(QWidget *parent, resParam *param);
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
