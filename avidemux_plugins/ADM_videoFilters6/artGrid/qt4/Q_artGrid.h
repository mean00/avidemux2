#ifndef Q_artGrid_h
#define Q_artGrid_h
#include "ui_artGrid.h"
#include "ADM_image.h"
#include "artGrid.h"
#include "DIA_flyDialogQt4.h"
#include "DIA_flyArtGrid.h"

class Ui_artGridWindow : public QDialog
{
    Q_OBJECT

  protected:
    int lock;
    flyArtGrid *     myFly;
    ADM_QCanvas *      canvas;
    Ui_artGridDialog ui;

  public:
    Ui_artGridWindow(QWidget *parent, artGrid *param,ADM_coreVideoFilter *in);
    ~Ui_artGridWindow();

  public slots:
    void gather(artGrid *param);

  private slots:
    void sliderUpdate(int foo);
    void valueChanged(int foo);
    void valueChangedSpinBox(int foo);
    void reset(void);

  private:
    void resizeEvent(QResizeEvent *event);
    void showEvent(QShowEvent *event);
};
#endif    // Q_artGrid_h
