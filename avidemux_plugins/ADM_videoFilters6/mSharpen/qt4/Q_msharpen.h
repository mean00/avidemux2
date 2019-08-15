#pragma once
#include "ui_msharpen.h"
#include "DIA_flyDialogQt4.h"
#include "ADM_image.h"
#include "msharpen.h"
#include "DIA_flymsharpen.h"
class Ui_msharpenWindow : public QDialog
{
        Q_OBJECT

protected: 
        int lock;

public:
        flyMSharpen *flymsharpen;
        ADM_QCanvas *canvas;
        Ui_msharpenWindow(QWidget *parent, msharpen *param, ADM_coreVideoFilter *in);
        ~Ui_msharpenWindow();
        Ui_msharpenDialog ui;

public slots:
        void gather(msharpen *param);

private slots:
        void sliderUpdate(int foo);
        void valueChanged(int foo);

private:
        void resizeEvent(QResizeEvent *event);
        void showEvent(QShowEvent *event);
};
