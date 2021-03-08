#ifndef Q_artMirror_h
#define Q_artMirror_h
#include "ui_artMirror.h"
#include "ADM_image.h"
#include "artMirror.h"
#include "DIA_flyDialogQt4.h"
#include "DIA_flyArtMirror.h"

class Ui_artMirrorWindow : public QDialog
{
    Q_OBJECT

  protected:
    int lock;
    flyArtMirror *     myFly;
    ADM_QCanvas *      canvas;
    Ui_artMirrorDialog ui;

  public:
    Ui_artMirrorWindow(QWidget *parent, artMirror *param,ADM_coreVideoFilter *in);
    ~Ui_artMirrorWindow();

  public slots:
    void gather(artMirror *param);

  private slots:
    void sliderUpdate(int foo);
    void valueChanged(int foo);

  private:
    void resizeEvent(QResizeEvent *event);
    void showEvent(QShowEvent *event);
};
#endif    // Q_artMirror_h
