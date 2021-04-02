#ifndef Q_colorTemp_h
#define Q_colorTemp_h
#include "ui_colorTemp.h"
#include "colorTemp.h"
#include "DIA_flyDialogQt4.h"
#include "DIA_flyColorTemp.h"

class Ui_colorTempWindow : public QDialog
{
    Q_OBJECT

  protected:
    int lock;
    flyColorTemp *     myFly;
    ADM_QCanvas *      canvas;
    Ui_colorTempDialog ui;

  public:
    Ui_colorTempWindow(QWidget *parent, colorTemp *param,ADM_coreVideoFilter *in);
    ~Ui_colorTempWindow();

  public slots:
    void gather(colorTemp *param);

  private slots:
    void sliderUpdate(int foo);
    void valueChanged(int foo);
    void valueChangedSpinBox(double foo);
    void reset(void);

  private:
    void resizeEvent(QResizeEvent *event);
    void showEvent(QShowEvent *event);
};
#endif    // Q_colorTemp_h
