#ifndef Q_asharp_h
#define Q_asharp_h

#include "ui_asharp.h"
#include "DIA_flyDialogQt4.h"
#include "ADM_image.h"
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
	Ui_asharpWindow(QWidget *parent, asharp *param, ADM_coreVideoFilter *in);
	~Ui_asharpWindow();
	Ui_asharpDialog ui;

public slots:
	void gather(asharp *param);

private slots:
	void sliderUpdate(int foo);
	void valueChanged(double foo);
	void valueChanged2(int foo);
	void valueChangedSlider(int foo);

private:
        void resizeEvent(QResizeEvent *event);
        void showEvent(QShowEvent *event);
};
#endif	// Q_asharp_h
