#ifndef Q_flip_h
#define Q_flip_h
#include "ui_flip.h"
#include "ADM_image.h"
#include "flip.h"
#include "DIA_flyDialogQt4.h"
#include "DIA_flyFlip.h"

class Ui_flipWindow : public QDialog
{
    Q_OBJECT

  protected:
    int lock;
    flyFlip *     myFly;
    ADM_QCanvas * canvas;
    Ui_flipDialog ui;

  public:
    Ui_flipWindow(QWidget *parent, flip *param,ADM_coreVideoFilter *in);
    ~Ui_flipWindow();

  public slots:
    void gather(flip *param);

  private slots:
    void sliderUpdate(int foo);
    void flipdirChanged(int foo);

  private:
    void resizeEvent(QResizeEvent *event);
    void showEvent(QShowEvent *event);
};
#endif    // Q_flip_h
