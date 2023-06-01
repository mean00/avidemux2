#ifndef Q_grain_h
#define Q_grain_h
#include "ui_grain.h"
#include "ADM_image.h"
#include "grain.h"
#include "DIA_flyDialogQt4.h"
#include "DIA_flyGrain.h"

class Ui_grainWindow : public QDialog
{
    Q_OBJECT

  protected:
    int lock;
    flyGrain *     myFly;
    ADM_QCanvas *   canvas;
    Ui_grainDialog ui;

  public:
    Ui_grainWindow(QWidget *parent, grain *param,ADM_coreVideoFilter *in);
    ~Ui_grainWindow();

  public slots:
    void gather(grain *param);

  private slots:
    void sliderUpdate(int foo);
    void valueChanged(int foo);
    void reset(void);

};
#endif    // Q_grain_h
