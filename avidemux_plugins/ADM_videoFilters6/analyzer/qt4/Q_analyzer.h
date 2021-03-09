#ifndef Q_analyzer_h
#define Q_analyzer_h
#include "ui_analyzer.h"
#include "DIA_flyDialogQt4.h"
#include "DIA_flyAnalyzer.h"

class Ui_analyzerWindow : public QDialog
{
    Q_OBJECT

  protected:
    bool firstRun;
    ADM_coreVideoFilter * _in;
    flyAnalyzer *     myFly;
    ADM_QCanvas *      canvas;
    Ui_analyzerDialog ui;
    QGraphicsScene * sceneVectorScope;
    QGraphicsScene * sceneYUVparade;
    QGraphicsScene * sceneRGBparade;
    QGraphicsScene * sceneHistograms;

  public:
    Ui_analyzerWindow(QWidget *parent, ADM_coreVideoFilter *in);
    ~Ui_analyzerWindow();

  private slots:
    void sliderUpdate(int foo);

  private:
    void resizeEvent(QResizeEvent *event);
    void showEvent(QShowEvent *event);
    void adjustGraphs(void);
};
#endif    // Q_analyzer_h
