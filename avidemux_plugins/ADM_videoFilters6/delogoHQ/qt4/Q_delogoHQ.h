#ifndef Q_delogoHQ_h
#define Q_delogoHQ_h
#include "ui_delogoHQ.h"
#include "ADM_image.h"
#include "delogoHQ.h"
#include "DIA_flyDialogQt4.h"
#include "DIA_flyDelogoHQ.h"

class Ui_delogoHQWindow : public QDialog
{
    Q_OBJECT

  protected:
    uint32_t       width,height;
    int            lock;
    std::string    lastFolder;
  public:
    flyDelogoHQ *myFly;
    ADM_QCanvas *canvas;
    Ui_delogoHQWindow(QWidget *parent, delogoHQ *param,ADM_coreVideoFilter *in);
    ~Ui_delogoHQWindow();
    Ui_delogoHQDialog ui;

    std::string         maskFName;

  public slots:
    void gather(delogoHQ *param);

  private slots:
    void sliderUpdate(int foo);
    void valueChanged(int foo);
    void valueChangedSpinBox(int foo);
    void showHelp();
    void imageSave();
    void imageLoad();

  private:
    void resizeEvent(QResizeEvent *event);
    void showEvent(QShowEvent *event);
    bool tryToLoadimage(const char *filename);
};
#endif    // Q_delogoHQ_h
