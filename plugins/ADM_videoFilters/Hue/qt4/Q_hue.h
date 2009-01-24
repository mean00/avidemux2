#ifndef Q_hue_h
#define Q_hue_h
#include "DIA_flyDialog.h"
#include "ui_hue.h"
#include "ADM_image.h"
#include "ADM_videoFilter.h"
#include "ADM_vidHue.h"

#include "DIA_flyDialogQt4.h"
#include "DIA_flyHue.h"

class Ui_hueWindow : public QDialog
{
	Q_OBJECT

protected:
	int lock;

public:
	flyHue *myCrop;
	ADM_QCanvas *canvas;
	Ui_hueWindow(Hue_Param *param,AVDMGenericVideoStream *in);
	~Ui_hueWindow();
	Ui_hueDialog ui;

public slots:
	void gather(Hue_Param *param);

private slots:
	void sliderUpdate(int foo);
	void valueChanged(int foo);
};
#endif	// Q_hue_h
