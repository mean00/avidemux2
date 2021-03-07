#ifndef Q_artVHS_h
#define Q_artVHS_h
#include "ui_artVHS.h"
#include "ADM_image.h"
#include "artVHS.h"
#include "DIA_flyDialogQt4.h"
#include "DIA_flyArtVHS.h"

class Ui_artVHSWindow : public QDialog
{
    Q_OBJECT

  protected:
    int lock;
    flyArtVHS *     myFly;
    ADM_QCanvas *   canvas;
    Ui_artVHSDialog ui;

  public:
    Ui_artVHSWindow(QWidget *parent, artVHS *param,ADM_coreVideoFilter *in);
    ~Ui_artVHSWindow();

  public slots:
    void gather(artVHS *param);

  private slots:
    void sliderUpdate(int foo);
    void valueChanged(int foo);
    void reset(void);

  private:
    void resizeEvent(QResizeEvent *event);
    void showEvent(QShowEvent *event);
};
#endif    // Q_artVHS_h
