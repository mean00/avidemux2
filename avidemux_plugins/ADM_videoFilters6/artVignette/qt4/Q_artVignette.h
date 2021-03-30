#ifndef Q_artVignette_h
#define Q_artVignette_h
#include "ui_artVignette.h"
#include "DIA_flyDialogQt4.h"
#include "DIA_flyArtVignette.h"

class Ui_artVignetteWindow : public QDialog
{
    Q_OBJECT

  protected:
    int lock;
    flyArtVignette *   myFly;
    ADM_QCanvas *      canvas;
    Ui_artVignetteDialog ui;

  public:
    Ui_artVignetteWindow(QWidget *parent, artVignette *param,ADM_coreVideoFilter *in);
    ~Ui_artVignetteWindow();

  public slots:
    void gather(artVignette *param);

  private slots:
    void sliderUpdate(int foo);
    void valueChanged(int foo);
    void valueChangedSpinBox(double foo);
    void reset(void);

  private:
    void resizeEvent(QResizeEvent *event);
    void showEvent(QShowEvent *event);
};
#endif    // Q_artVignette_h
