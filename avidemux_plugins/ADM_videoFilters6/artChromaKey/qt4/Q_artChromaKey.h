#ifndef Q_artChromaKey_h
#define Q_artChromaKey_h
#include "ui_artChromaKey.h"
#include "ADM_image.h"
#include "artChromaKey.h"
#include "DIA_flyDialogQt4.h"
#include "DIA_flyArtChromaKey.h"
#include <QGraphicsScene>

class Ui_artChromaKeyWindow : public QDialog
{
    Q_OBJECT

  protected:
    int lock;
    std::string            lastFolder;
    flyArtChromaKey *      myFly;
    ADM_QCanvas *          canvas;
    QGraphicsScene *       scene;
    Ui_artChromaKeyDialog  ui;

    bool                   tryToLoadimage(const char *filename);

  public:
    Ui_artChromaKeyWindow(QWidget *parent, artChromaKey *param,ADM_coreVideoFilter *in);
    ~Ui_artChromaKeyWindow();

    std::string         imageName;
    ADMImage            *image;
    ADMImage            *testimage;

  public slots:
    void gather(artChromaKey *param);

  private slots:
    void sliderUpdate(int foo);
    void valueChanged(int foo);
    void testImageChanged(int foo);
    void pushedC1();
    void pushedC2();
    void pushedC3();
    void imageSelect();


  private:
    void resizeEvent(QResizeEvent *event);
    void showEvent(QShowEvent *event);
};
#endif    // Q_artChromaKey_h
