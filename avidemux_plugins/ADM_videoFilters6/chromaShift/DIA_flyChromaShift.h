#ifndef FLY_CHROMASHIFT_H
#define FLY_CHROMASHIFT_H
#include "chromashift.h"
/**
    \class flyChromaShift
*/  
class flyChromaShift : public ADM_flyDialogYuv
{
  
  public:
   chromashift     param;
  public:
    uint8_t    processYuv(ADMImage* in, ADMImage *out);

   uint8_t    download(void);
   uint8_t    upload(void);
   uint8_t    update(void);
   flyChromaShift (QDialog *parent,uint32_t width,uint32_t height,ADM_coreVideoFilter *in,
                                    ADM_QCanvas *canvas, ADM_QSlider *slider)
                : ADM_flyDialogYuv(parent,width, height,in,canvas, slider,RESIZE_AUTO) {};
};
#endif
//EOF
