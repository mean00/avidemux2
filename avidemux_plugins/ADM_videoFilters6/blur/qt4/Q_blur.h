#ifndef Q_blur_h
#define Q_blur_h
#include "ui_blur.h"
#include "ADM_image.h"
#include "blur.h"
#include "DIA_flyDialogQt4.h"
#include "DIA_flyBlur.h"

class Ui_blurWindow : public QDialog
{
    Q_OBJECT

  protected:
    int lock;

  public:
    flyBlur *myFly;
    ADM_QCanvas *canvas;
    Ui_blurWindow(QWidget *parent, blur *param,ADM_coreVideoFilter *in);
    ~Ui_blurWindow();
    Ui_blurDialog ui;

  public slots:
    void gather(blur *param);

  private slots:
    void sliderUpdate(int foo);
    void valueChanged(int foo);
    void valueChangedSpinBox(int foo);
    void reset(bool f);
    void toggleRubber(int checkState);

  private:
    void resizeEvent(QResizeEvent *event);
    void showEvent(QShowEvent *event);
};
#endif    // Q_blur_h
