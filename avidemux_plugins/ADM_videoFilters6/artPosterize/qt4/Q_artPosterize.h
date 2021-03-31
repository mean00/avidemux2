#ifndef Q_artPosterize_h
#define Q_artPosterize_h
#include "ui_artPosterize.h"
#include "ADM_image.h"
#include "artPosterize.h"
#include "DIA_flyDialogQt4.h"
#include "DIA_flyArtPosterize.h"

class Ui_artPosterizeWindow : public QDialog
{
    Q_OBJECT

  protected:
    int lock;

  public:
    flyArtPosterize *myFly;
    ADM_QCanvas *canvas;
    Ui_artPosterizeWindow(QWidget *parent, artPosterize *param,ADM_coreVideoFilter *in);
    ~Ui_artPosterizeWindow();
    Ui_artPosterizeDialog ui;

  public slots:
    void gather(artPosterize *param);

  private slots:
    void sliderUpdate(int foo);
    void valueChanged(int foo);
    void valueChangedSpinBox(int foo);

  private:
    void resizeEvent(QResizeEvent *event);
    void showEvent(QShowEvent *event);
};
#endif    // Q_artPosterize_h
