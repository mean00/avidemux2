#include "DIA_flyDialog.h"
#include "ui_chromashift.h"
#include "ADM_image.h"
#include "DIA_flyDialogQt4.h"
#include "DIA_flyChromaShift.h"
#include "chromashift.h"



#include <math.h>

class Ui_chromaShiftWindow : public QDialog
 {
     Q_OBJECT
 protected : 
    int lock;
 public:
     flyChromaShift *myCrop;
     ADM_QCanvas *canvas;
     Ui_chromaShiftWindow(QWidget* parent, chromashift *param,ADM_coreVideoFilter *in);
     ~Ui_chromaShiftWindow();
     Ui_chromashiftDialog ui;
 public slots:
      void gather(chromashift *param);
      //void update(int i);
 private slots:
   void sliderUpdate(int foo);
   void valueChanged(int foo);

 private:
     
 };

