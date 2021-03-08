#ifndef Q_artDynThreshold_h
#define Q_artDynThreshold_h
#include "ui_artDynThreshold.h"
#include "ADM_image.h"
#include "artDynThreshold.h"
#include "DIA_flyDialogQt4.h"
#include "DIA_flyArtDynThreshold.h"

class Ui_artDynThresholdWindow : public QDialog
{
    Q_OBJECT

  protected:
    int lock;
    flyArtDynThreshold *     myFly;
    ADM_QCanvas *      canvas;
    Ui_artDynThresholdDialog ui;

  public:
    Ui_artDynThresholdWindow(QWidget *parent, artDynThreshold *param,ADM_coreVideoFilter *in);
    ~Ui_artDynThresholdWindow();

  public slots:
    void gather(artDynThreshold *param);

  private slots:
    void sliderUpdate(int foo);
    void valueChanged(int foo);
    void reset(void);

  private:
    void resizeEvent(QResizeEvent *event);
    void showEvent(QShowEvent *event);
};
#endif    // Q_artDynThreshold_h
