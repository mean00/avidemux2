#pragma once
#include "DIA_flyDialogQt4.h"
#include "asharp.h"
/**
    \class flyASharp
*/
class flyASharp : public ADM_flyDialogYuv
{
  
  public:
   asharp     param;
  public:
    uint8_t    processYuv(ADMImage* in, ADMImage *out);
    void       blockChanges(bool block);
    void       setTabOrder(void);

   uint8_t    download(void);
   uint8_t    upload(void);
   uint8_t    update(void);
                flyASharp (QDialog *parent,uint32_t width,uint32_t height,ADM_coreVideoFilter *in,
                                    ADM_QCanvas *canvas, ADM_QSlider *slider) :
                    ADM_flyDialogYuv(parent,width, height,in,canvas, slider,RESIZE_AUTO) { }
};

