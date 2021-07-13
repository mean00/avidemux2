#ifndef Q_imageStab_h
#define Q_imageStab_h
#include "ui_imageStab.h"
#include "imageStab.h"
#include "DIA_flyImageStab.h"

class Ui_imageStabWindow : public QDialog
{
    Q_OBJECT

  protected:
    int lock;
    flyImageStab *myFly;
    ADM_QCanvas *canvas;
    Ui_imageStabDialog ui;

  public:
    Ui_imageStabWindow(QWidget *parent, imageStab *param,ADM_coreVideoFilter *in);
    ~Ui_imageStabWindow();

  public slots:
    void gather(imageStab *param);

  private slots:
    void sliderUpdate(int foo);
    void valueChanged(int foo);
    void reset(bool f);

  private:
    void resizeEvent(QResizeEvent *event);
    void showEvent(QShowEvent *event);
};
#endif    // Q_imageStab_h
