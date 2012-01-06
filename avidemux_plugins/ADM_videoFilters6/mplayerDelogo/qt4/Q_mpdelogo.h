#ifndef Q_mpdelogo_h
#define Q_mpdelogo_h

#include "ui_mpdelogo.h"
#include "DIA_flyDialog.h"
#include "ADM_image.h"
#include "DIA_flyDialogQt4.h"
#include "DIA_flyMpDelogo.h"

class Ui_mpdelogoWindow : public QDialog
{
	Q_OBJECT

protected: 
	int lock;

public:
	flyMpDelogo *myCrop;
	ADM_QCanvas *canvas;
	Ui_mpdelogoWindow(QWidget *parent, delogo *param, ADM_coreVideoFilter *in);
	~Ui_mpdelogoWindow();
	Ui_mpdelogoDialog ui;

public slots:
	void gather(delogo *param);

private slots:
	void sliderUpdate(int foo);
	void valueChanged(int foo);
};
#endif
