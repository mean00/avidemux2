
#include "DIA_flyDialog.h"
#include "ADM_vidChromaShift_param.h"
#include "DIA_flyChromaShift.h"

#include <math.h>

#include "ui_chromashift.h"
class Ui_chromaShiftWindow : public QDialog
 {
     Q_OBJECT
 protected : 
    int lock;
 public:
     flyChromaShift *myCrop;
     ADM_QCanvas *canvas;
     Ui_chromaShiftWindow(CHROMASHIFT_PARAM *param,AVDMGenericVideoStream *in);
     ~Ui_chromaShiftWindow();
     Ui_chromashiftDialog ui;
 public slots:
      void gather(CHROMASHIFT_PARAM *param);
      //void update(int i);
 private slots:
   void sliderUpdate(int foo);
   void valueChanged(int foo);

 private:
     
 };

