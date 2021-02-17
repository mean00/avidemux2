#ifndef Q_analyzer_h
#define Q_analyzer_h
#include "ui_analyzer.h"
#include "ADM_image.h"
#include "DIA_flyDialogQt4.h"
#include "DIA_flyAnalyzer.h"
#include "QGraphicsScene"

class Ui_analyzerWindow : public QDialog
{
    Q_OBJECT

  protected:
    int lock;
    uint32_t         _width, _height;
    ADM_coreVideoFilter * _in;
    flyAnalyzer *     myFly;
    ADM_QCanvas *      canvas;
    Ui_analyzerDialog ui;
    QGraphicsScene * sceneVectorScope;
    QGraphicsScene * sceneYUVparade;
    QGraphicsScene * sceneRGBparade;
    QGraphicsScene * sceneHystograms;

  public:
    Ui_analyzerWindow(QWidget *parent, ADM_coreVideoFilter *in);
    ~Ui_analyzerWindow();

  private slots:
    void sliderUpdate(int foo);

  private:
    void resizeEvent(QResizeEvent *event);
    void showEvent(QShowEvent *event);
};
#endif    // Q_analyzer_h
