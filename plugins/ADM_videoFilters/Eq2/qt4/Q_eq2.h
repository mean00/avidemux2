#ifndef Q_eq2_h
#define Q_eq2_h
#include "DIA_flyDialog.h"
#include "ui_eq2.h"
#include "ADM_image.h"

#include "ADM_vidEq2.h"
#include "DIA_flyEq2.h"

class Ui_eq2Window : public QDialog
{
	Q_OBJECT

protected:
	int lock;

public:
	flyEq2 *myCrop;
	ADM_QCanvas *canvas;
	Ui_eq2Window(Eq2_Param *param,AVDMGenericVideoStream *in);
	~Ui_eq2Window();
	Ui_eq2Dialog ui;

public slots:
	void gather(Eq2_Param *param);

private slots:
	void sliderUpdate(int foo);
	void valueChanged(int foo);
};
#endif	// Q_eq2_h
