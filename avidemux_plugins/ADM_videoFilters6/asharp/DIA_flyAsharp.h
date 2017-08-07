
#pragma once
/**
    \class flyASharp
*/
#include "asharp.h"
#define uc uint8_t
class flyASharp : public ADM_flyDialogYuv
{
  
  public:
   asharp     param;
  public:
    uint8_t    processYuv(ADMImage* in, ADMImage *out);

   uint8_t    download(void);
   uint8_t    upload(void);
   uint8_t    update(void);
                flyASharp (QDialog *parent,uint32_t width,uint32_t height,ADM_coreVideoFilter *in,
                                    ADM_QCanvas *canvas, ADM_QSlider *slider) :
                    ADM_flyDialogYuv(parent,width, height,in,canvas, slider,RESIZE_AUTO) {};
};

void asharp_run_c(      uc* planeptr, int pitch,
                                        int height, int width, 
                                        int     T,int D, int B, int B2, bool bf,uint8_t *line );



