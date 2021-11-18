#ifndef Q_cubicLUT_h
#define Q_cubicLUT_h
#include "ui_cubicLUT.h"
#include "ADM_image.h"
#include "cubicLUT.h"
#include "DIA_flyDialogQt4.h"
#include "DIA_flyCubicLUT.h"

class Ui_cubicLUTWindow : public QDialog
{
    Q_OBJECT

  protected:
    uint32_t       width,height;
    std::string    lastFolder;
  public:
    flyCubicLUT *myFly;
    ADM_QCanvas *canvas;
    Ui_cubicLUTWindow(QWidget *parent, cubicLUT *param,ADM_coreVideoFilter *in);
    ~Ui_cubicLUTWindow();
    Ui_cubicLUTDialog ui;

    std::string         lutFName;

  public slots:
    void gather(cubicLUT *param);
    void okButtonClicked();

  private slots:
    void sliderUpdate(int foo);
    void imageLoad();
    void cubeLoad();

  private:
    void resizeEvent(QResizeEvent *event);
    void showEvent(QShowEvent *event);
    bool tryToLoadImage(const char *filename);
    bool tryToLoadCube(const char *filename);
};
#endif    // Q_cubicLUT_h
