#pragma once
#include "ui_mpdelogo.h"
#include "ADM_image.h"
#include "DIA_flyDialogQt4.h"
#include "DIA_flyMpDelogo.h"

/**
 * 
 * @return 
 */
class Ui_mpdelogoWindow : public QDialog
{
        Q_OBJECT

public:
                            Ui_mpdelogoWindow(QWidget *parent, delogo *param, ADM_coreVideoFilter *in);
                            ~Ui_mpdelogoWindow();

private:
        int                 lock;
        Ui_mpdelogoDialog   ui;
        QHBoxLayout         *helpLayout;
        ADM_coreVideoFilter *_in;
        flyMpDelogo         *myCrop;
        ADM_QCanvas         *canvas;

        void                resizeEvent(QResizeEvent *event);
        void                showEvent(QShowEvent *event);
        void                setSpinWidth(int w, int h);

public slots:
        void                gather(delogo *param);

private slots:
        void                sliderUpdate(int foo);
        void                valueChanged(int foo);
        void                sizeChanged(int foo);
        void                preview(int x);
};

