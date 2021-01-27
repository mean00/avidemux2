#ifndef Q_artPixelize_h
#define Q_artPixelize_h
#include "ui_artPixelize.h"
#include "ADM_image.h"
#include "artPixelize.h"
#include "DIA_flyDialogQt4.h"
#include "DIA_flyArtPixelize.h"

class Ui_artPixelizeWindow : public QDialog
{
    Q_OBJECT

  protected:
    int lock;

  public:
    flyArtPixelize *     myFly;
    ADM_QCanvas *      canvas;
    Ui_artPixelizeWindow(QWidget *parent, artPixelize *param,ADM_coreVideoFilter *in);
    ~Ui_artPixelizeWindow();
    Ui_artPixelizeDialog ui;

  public slots:
    void gather(artPixelize *param);

  private slots:
    void sliderUpdate(int foo);
    void valueChanged(int foo);

  private:
    void resizeEvent(QResizeEvent *event);
    void showEvent(QShowEvent *event);
};
#endif    // Q_artPixelize_h
