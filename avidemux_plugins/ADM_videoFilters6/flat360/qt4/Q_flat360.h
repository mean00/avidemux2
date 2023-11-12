#ifndef Q_flat360_h
#define Q_flat360_h
#include "ui_flat360.h"
#include "ADM_image.h"
#include "flat360.h"
#include "DIA_flyDialogQt4.h"
#include "DIA_flyFlat360.h"

class Ui_flat360Window : public QDialog
{
    Q_OBJECT

  protected:
    int lock;

  public:
    flyFlat360 *myFly;
    ADM_QCanvas *canvas;
    Ui_flat360Window(QWidget *parent, flat360 *param,ADM_coreVideoFilter *in);
    ~Ui_flat360Window();
    Ui_flat360Dialog ui;

  public slots:
    void gather(flat360 *param);

  private slots:
    void sliderUpdate(int foo);
    void valueChanged(int foo);
    void valueChangedSpinBox(double foo);
    void reset(bool f);
};
#endif    // Q_flat360_h
