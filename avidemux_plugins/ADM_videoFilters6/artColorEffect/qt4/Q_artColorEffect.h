#ifndef Q_artColorEffect_h
#define Q_artColorEffect_h
#include "ui_artColorEffect.h"
#include "ADM_image.h"
#include "artColorEffect.h"
#include "DIA_flyDialogQt4.h"
#include "DIA_flyArtColorEffect.h"

class Ui_artColorEffectWindow : public QDialog
{
    Q_OBJECT

  protected:
    int lock;

  public:
    flyArtColorEffect *myFly;
    ADM_QCanvas *canvas;
    Ui_artColorEffectWindow(QWidget *parent, artColorEffect *param,ADM_coreVideoFilter *in);
    ~Ui_artColorEffectWindow();
    Ui_artColorEffectDialog ui;

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
