#ifndef Q_artCharcoal_h
#define Q_artCharcoal_h
#include "ui_artCharcoal.h"
#include "ADM_image.h"
#include "artCharcoal.h"
#include "DIA_flyDialogQt4.h"
#include "DIA_flyArtCharcoal.h"

class Ui_artCharcoalWindow : public QDialog
{
    Q_OBJECT

  protected:
    int lock;
    flyArtCharcoal *     myFly;
    ADM_QCanvas *      canvas;
    Ui_artCharcoalDialog ui;

  public:
    Ui_artCharcoalWindow(QWidget *parent, artCharcoal *param,ADM_coreVideoFilter *in);
    ~Ui_artCharcoalWindow();

  public slots:
    void gather(artCharcoal *param);

  private slots:
    void sliderUpdate(int foo);
    void valueChanged(int foo);
    void reset(void);

  private:
    void resizeEvent(QResizeEvent *event);
    void showEvent(QShowEvent *event);
};
#endif    // Q_artCharcoal_h
