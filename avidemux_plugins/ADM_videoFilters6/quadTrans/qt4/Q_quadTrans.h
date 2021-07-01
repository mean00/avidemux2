#ifndef Q_quadTrans_h
#define Q_quadTrans_h
#include "ui_quadTrans.h"
#include "ADM_image.h"
#include "quadTrans.h"
#include "DIA_flyDialogQt4.h"
#include "DIA_flyQuadTrans.h"

class Ui_quadTransWindow : public QDialog
{
    Q_OBJECT

  protected:
    int lock;

  public:
    flyQuadTrans *myFly;
    ADM_QCanvas *canvas;
    Ui_quadTransWindow(QWidget *parent, quadTrans *param,ADM_coreVideoFilter *in);
    ~Ui_quadTransWindow();
    Ui_quadTransDialog ui;

  public slots:
    void gather(quadTrans *param);

  private slots:
    void sliderUpdate(int foo);
    void valueChanged(int foo);
    void valueChangedSpinBox(double foo);
    void reset(bool f);

  private:
    void resizeEvent(QResizeEvent *event);
    void showEvent(QShowEvent *event);
};
#endif    // Q_quadTrans_h
