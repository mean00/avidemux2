
#pragma once
#include "QHBoxLayout"
#include "QSizeGrip"
#include "QRubberBand"
class flyCrop;
/**
        \class Resizable_rubber_band
        \brief http://stackoverflow.com/questions/19066804/implementing-resize-handles-on-qrubberband-is-qsizegrip-relevant
*/
class cropRubber : public QWidget 
{
public:
                cropRubber(flyCrop *fly,QWidget* parent = 0);
  flyCrop       *flyParent;
  int           nestedIgnore;

public:
  QRubberBand* rubberband;
  void resizeEvent(QResizeEvent *);
  void blockSignals(bool sig)
  {
      rubberband->blockSignals(sig);
  }  
};

class flyCrop : public ADM_flyDialogRgb
{
  
  public:
   uint32_t left,right,top,bottom;
  public:
   uint8_t    processRgb(uint8_t *imageIn, uint8_t *imageOut);
   uint8_t    download(void);
   uint8_t    upload(void) {return upload(true);}
   uint8_t    upload(bool redraw);
   uint8_t    autocrop(void);
   bool       bandResized(int x,int y,int w, int h);
              flyCrop (QDialog *parent,uint32_t width,uint32_t height,ADM_coreVideoFilter *in,
                                    ADM_QCanvas *canvas, QSlider *slider);
    virtual   ~flyCrop();
protected:
    cropRubber  *rubber;
    bool        blockChanges(bool block);
    int         autoRun(uint8_t *in,int w,int h, int increment);
};

