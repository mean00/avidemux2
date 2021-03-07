#ifndef Q_artColorEffect_h
#define Q_artColorEffect_h
#include "ui_artColorEffect.h"
#include "artColorEffect.h"
#include "DIA_flyDialogQt4.h"
#include "DIA_flyArtColorEffect.h"

class Ui_artColorEffectWindow : public QDialog
{
    Q_OBJECT

  protected:
    int lock;
    flyArtColorEffect *myFly;
    ADM_QCanvas *canvas;
    Ui_artColorEffectDialog ui;

  public:
    Ui_artColorEffectWindow(QWidget *parent, artColorEffect *param,ADM_coreVideoFilter *in);
    ~Ui_artColorEffectWindow();

  public slots:
    void gather(artColorEffect *param);

  private slots:
    void sliderUpdate(int foo);
    void effectChanged(int foo);

  private:
    void resizeEvent(QResizeEvent *event);
    void showEvent(QShowEvent *event);
};
#endif    // Q_artColorEffect_h
