#ifndef Q_artChromaHold_h
#define Q_artChromaHold_h
#include "ui_artChromaHold.h"
#include "ADM_image.h"
#include "artChromaHold.h"
#include "DIA_flyDialogQt4.h"
#include "DIA_flyArtChromaHold.h"
#include <QGraphicsScene>

class Ui_artChromaHoldWindow : public QDialog
{
    Q_OBJECT

  protected:
    int lock;
    flyArtChromaHold *     myFly;
    ADM_QCanvas *      canvas;
    Ui_artChromaHoldDialog ui;
    QGraphicsScene *scene;

  public:
    Ui_artChromaHoldWindow(QWidget *parent, artChromaHold *param,ADM_coreVideoFilter *in);
    ~Ui_artChromaHoldWindow();

  public slots:
    void gather(artChromaHold *param);

  private slots:
    void sliderUpdate(int foo);
    void valueChanged(int foo);
    void pushedC1();
    void pushedC2();
    void pushedC3();


  private:
    void resizeEvent(QResizeEvent *event);
    void showEvent(QShowEvent *event);
};
#endif    // Q_artChromaHold_h
