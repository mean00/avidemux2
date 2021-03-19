#ifndef Q_artCartoon_h
#define Q_artCartoon_h
#include "ui_artCartoon.h"
#include "ADM_image.h"
#include "artCartoon.h"
#include "DIA_flyDialogQt4.h"
#include "DIA_flyArtCartoon.h"

class Ui_artCartoonWindow : public QDialog
{
    Q_OBJECT

  protected:
    int lock;
    flyArtCartoon *myFly;
    ADM_QCanvas *canvas;
    Ui_artCartoonDialog ui;

  public:
    Ui_artCartoonWindow(QWidget *parent, artCartoon *param,ADM_coreVideoFilter *in);
    ~Ui_artCartoonWindow();

  public slots:
    void gather(artCartoon *param);

  private slots:
    void sliderUpdate(int foo);
    void valueChanged(int foo);

  private:
    void resizeEvent(QResizeEvent *event);
    void showEvent(QShowEvent *event);
};
#endif    // Q_artCartoon_h
