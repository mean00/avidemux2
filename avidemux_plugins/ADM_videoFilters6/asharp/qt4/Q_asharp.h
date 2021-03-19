#ifndef Q_asharp_h
#define Q_asharp_h

#include "ui_asharp.h"
#include "DIA_flyAsharp.h"

class Ui_asharpWindow : public QDialog
{
    Q_OBJECT

protected:
    int                 lock;
    Ui_asharpDialog     ui;
    flyASharp           *myCrop;
    ADM_QCanvas         *canvas;

public:
    Ui_asharpWindow(QWidget *parent, asharp *param, ADM_coreVideoFilter *in);
    ~Ui_asharpWindow();

public slots:
    void gather(asharp *param);

private slots:
    void sliderUpdate(int foo);
    void valueChanged(double foo);
    void valueChanged2(int foo);
    void valueChangedSlider(int foo);
    void reset(void);

private:
    void resizeEvent(QResizeEvent *event);
    void showEvent(QShowEvent *event);
};
#endif	// Q_asharp_h
