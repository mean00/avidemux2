#ifndef Q_fadeFromImage_h
#define Q_fadeFromImage_h
#include "ui_fadeFromImage.h"
#include "fadeFromImage.h"
#include "DIA_flyFadeFromImage.h"

class Ui_fadeFromImageWindow : public QDialog
{
    Q_OBJECT

  protected:
    int lock;
    uint64_t markerA, markerB, duration;
    flyFadeFromImage *myFly;
    ADM_QCanvas *canvas;
    Ui_fadeFromImageDialog ui;

  public:
    Ui_fadeFromImageWindow(QWidget *parent, fadeFromImage *param,ADM_coreVideoFilter *in);
    ~Ui_fadeFromImageWindow();

  public slots:
    void gather(fadeFromImage *param);

  private slots:
    void sliderUpdate(int foo);
    void manualTimeEntry(bool f);
    void timesFromMarkers(bool f);
    void valueChanged(int foo);
    void reset(bool f);

  private:
    void resizeEvent(QResizeEvent *event);
    void showEvent(QShowEvent *event);
};
#endif    // Q_fadeFromImage_h
