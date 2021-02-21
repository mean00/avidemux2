#ifndef Q_lumaStab_h
#define Q_lumaStab_h
#include "ui_lumaStab.h"
#include "lumaStab.h"
#include "DIA_flyDialogQt4.h"
#include "DIA_flyLumaStab.h"

class Ui_lumaStabWindow : public QDialog
{
    Q_OBJECT

  protected:
    int lock;
    flyLumaStab *     myFly;
    ADM_QCanvas *      canvas;
    Ui_lumaStabDialog ui;

  public:
    Ui_lumaStabWindow(QWidget *parent, lumaStab *param,ADM_coreVideoFilter *in);
    ~Ui_lumaStabWindow();

  public slots:
    void gather(lumaStab *param);

  private slots:
    void sliderUpdate(int foo);
    void valueChanged(int foo);
    void reset(void);

  private:
    void resizeEvent(QResizeEvent *event);
    void showEvent(QShowEvent *event);
};
#endif    // Q_lumaStab_h
