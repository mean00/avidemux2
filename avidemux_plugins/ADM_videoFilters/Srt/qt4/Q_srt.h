#ifndef Q_srt_h
#define Q_srt_h

#include "DIA_flyDialog.h"
#include "ui_srt.h"
#include "ADM_image.h"

class ADMfont;

#include "ADM_videoFilter.h"
#include "ADM_vidSRT.h"
#include "DIA_flyDialog.h"
#include "DIA_flySrtPos.h"

class Ui_srtWindow : public QDialog
{
	Q_OBJECT

protected:
	int lock;

public:
	flySrtPos *myCrop;
	ADM_QCanvas *canvas;
	Ui_srtWindow(QWidget *parent, SRT_POS_PARAM *param, AVDMGenericVideoStream *in);
	~Ui_srtWindow();
	Ui_srtDialog ui;

public slots:
	void gather(SRT_POS_PARAM *param);

private slots:
	void sliderUpdate(int foo);
	void valueChanged(int foo);
};
#endif	// Q_srt_h
