#ifndef Q_contrast_h
#define Q_contrast_h
#include "DIA_flyDialog.h"
#include "ui_contrast.h"
#include "ADM_image.h"
#include "ADM_videoFilter.h"
#include "ADM_vidContrast.h"


#include "DIA_flyContrast.h"

class Ui_contrastWindow : public QDialog
{
	Q_OBJECT

protected : 
	int lock;

public:
	flyContrast *myCrop;
	ADM_QCanvas *canvas;
	Ui_contrastWindow(QWidget* parent, CONTRAST_PARAM *param,AVDMGenericVideoStream *in);
	~Ui_contrastWindow();
	Ui_contrastDialog ui;

public slots:
	void gather(CONTRAST_PARAM *param);

private slots:
	void sliderUpdate(int foo);
	void valueChanged(int foo);
};
#endif	// Q_contrast_h
