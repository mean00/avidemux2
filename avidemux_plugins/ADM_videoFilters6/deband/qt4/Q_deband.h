#ifndef Q_deband_h
#define Q_deband_h
#include "ui_deband.h"
#include "ADM_image.h"
#include "deband.h"
#include "DIA_flyDialogQt4.h"
#include "DIA_flyDeband.h"

class Ui_debandWindow : public QDialog
{
    Q_OBJECT

  protected:
    int lock;
    flyDeband *     myFly;
    ADM_QCanvas *      canvas;
    Ui_debandDialog ui;

  public:
    Ui_debandWindow(QWidget *parent, deband *param,ADM_coreVideoFilter *in);
    ~Ui_debandWindow();

  public slots:
    void gather(deband *param);

  private slots:
    void sliderUpdate(int foo);
    void valueChanged(int foo);
    void valueChangedSpinBox(int foo);
    void reset(void);

  private:
    void resizeEvent(QResizeEvent *event);
    void showEvent(QShowEvent *event);
};
#endif    // Q_deband_h
