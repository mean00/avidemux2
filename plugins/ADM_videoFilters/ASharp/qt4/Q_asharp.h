#ifndef Q_asharp_h
#define Q_asharp_h

#include "ui_asharp.h"
#include "ADM_image.h"
#include "ADM_videoFilter.h"
#include "ADM_vidASharp_param.h"
#include "DIA_flyDialog.h"
#include "DIA_flyDialogQt4.h"
#include "DIA_flyAsharp.h"

class Ui_asharpWindow : public QDialog
{
	Q_OBJECT

protected: 
	int lock;

public:
	flyASharp *myCrop;
	ADM_QCanvas *canvas;
	Ui_asharpWindow(ASHARP_PARAM *param, AVDMGenericVideoStream *in);
	~Ui_asharpWindow();
	Ui_asharpDialog ui;

public slots:
	void gather(ASHARP_PARAM *param);

private slots:
	void sliderUpdate(int foo);
	void valueChanged(int foo);
};
#endif	// Q_asharp_h
